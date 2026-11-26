// Game input actions, the pointer cursor (drawing, colors and trails) and controller rumble.
// Input_MapActionToCategory and Input_IsActionPressed read controller state for game actions.
// InputManager_DrawCursors draws the cursor, InputManager_LoadCursorResources loads its images,
// and InputManager_StartRumble and its siblings drive the controller motors.

#include <decomp/ct_member.h>
// Game input actions, Wii Remote cursor drawing, trails and rumble.
// Engine input providers (input_device_drivers.cpp/gtl_pad_drivers_gc_classic_freestyle.cpp) supply InputEx categories;
// callers query 39 game actions through Input_MapActionToCategory below.
// Modding lookup:
// - Classic Controller support: provider reports are upstream; this mapper
//   aliases game actions onto categories. Preserve synthetic-press priority.
// - Widescreen pointer/label/trail: InputManager_DrawCursors dispatches both
//   draw paths; their rectangles are screen pixels, +X right/+Y down, and UVs
//   are normalized. Match the pointer transform to the active 2D projection.
// - Cursor size/atlas crop: kCursor*Pixels and kCursor*Uv* definitions live in
//   the data section. Increasing a pixel size changes geometry only, not UVs.
// - Click cadence: kCursorClickFrames counts DRAWS; rumble counts Update calls.
// - Bounds/resource paths: CursorResourcePool in LoadCursorResources, NOT a
//   local screen-resolution switch. Texture decoding chooses formats upstream.
// Render calls bind stage 0 and immediate quad format without restoring them;
// the caller must establish its next draw state. Borrowed input/layout handles
// remain engine-owned; LoadCursorResources replaces manager-owned allocations.
// Decoded images carry owned graphics references; see the load/reload contract
// below for the existing missing-release behavior (there is no image cache here).
extern "C" {
extern unsigned char s_cursorResourcePaths[];              // Cursor and player-number texture paths.
extern unsigned char s_cursorLabelPathsAndQuadTemplates[]; // Cursor label paths and quad templates.
}
#include "classes/CMouseCursor.h"
#include "MSL/vector.h"
namespace std {
template <> struct __vec_destroys_elements<void*> {
    enum {
        value = 5
    };
};
}
typedef std::vector<void*, app::app_allocator<void*> > CursorPointerVector;
#include "classes/CSharedDoc.h"
#include "classes/GameInputManager.h"
// Borrow the manager's field view; accepting byte cursors preserves shifted-array accesses.
static inline GameInputManager* AsGameInputManager(void* object) {
    return (GameInputManager*)object;
}
#include "classes/CGolfDoc.h"
#include "classes/CHashPath.h"
// Borrow the golf document's player/channel mapping; do not retain a new owner.
static inline CGolfDoc* AsCGolfDoc(void* object) {
    return (CGolfDoc*)object;
}
// API index/units contract:
// `player` normally means Wii Remote channel [0,4), not golf-player order.
// Only the GetPointerPosition/GetCursorDepth and action-query APIs explicitly
// substitute selectedPlayer for a negative channel. Direct draw/color/options
// entry points still index arrays with player and require a valid channel.
// Action query IDs must be [0,39); the mapper alone defaults unknown IDs to 0.
// XY and cursor extents are screen pixels; texture UVs are normalized fractions.
// Rumble durations count InputManager_Update calls; zero means stop pending,
// -1 inactive. Pause stops motors and freezes countdowns without discarding them.
// pointerOverride is copied in and consumed by the first cursor draw, not query.
// GameInputManager owns two pointer vectors (cursorLayouts, cursorTrails) with their counts;
// physical controller lookup goes through gtl_InputEx_GetScreenPointer.
// Layout views for the resource region omitted by GameInputManager's partial header.
// Three native words copied by DrawCursors. enabled selects this override;
// labelMode is 0 cursor-only/1 channel-label/2 number. playerColor must be 0..3:
// it selects BOTH the cursor color palette (UpdateCursorColor) and the paired
// numberTextures/numberPalettes slot (DrawPlayerCursorWithOptions). It is not
// the controller channel, an RGB value, or an appearance index. The setter
// stores it unchecked. LoadCursorResources fills exactly four numbered slots
// from CursorResourcePool::numberPaths in the same order; a replacement must
// preserve each image/palette pair. No channel-to-color default is inferred.
struct CursorDrawOptions {
    int enabled, labelMode, playerColor;
};
struct CursorColorOverride {
    int enabled;
    unsigned char red, green, blue, padding;
};
struct CursorResources {
    char prefix[0x2F4];
    CursorDrawOptions drawOptions[4];
    CursorColorOverride colorOverrides[4];
    void* cursorTextures[6];
    void* cursorPalettes[6];
    void* numberTextures[4];
    void* numberPalettes[4];
    void* labelTextures[4];
    void* labelPalettes[4];
};
// InputEx's screen object supplies the low-level device used for motor requests.
struct ScreenInput {
    char prefix[8];
    void* motorDevice;
};
// The cursor trail owns its allocations; colors contains two consecutive RGBA values.
struct CursorTrailStorage {
    char prefix[0x18];
    void* samples;
    void** sourceVertexArrays;
    void** animatedVertexArrays; // Both arrays contain vertex triplets, not index buffers.
    void* pointAngles;
    char reserved28[4];
    unsigned char colors[8];
    char reserved34[0x14];
    int arrayCount;
};
// Both vector specializations are (data, element count, capacity in elements).
// They own only the pointer array. Manager teardown separately destroys pointed
// layouts/trails; shrinking these vectors only changes size, without deleting
// payloads. Reserve swaps the old allocation into an unwind guard for release.
struct PointerVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};
struct WordIterator {
    unsigned int* current;
};
// The vector growth limits are the original 32-bit element-count tiers.
enum {
    kVectorMaxElements = 0x3FFFFFFF,
    kVectorSmallTier = 0x15555555,
    kVectorLargeTier = 0x2AAAAAAA
};
enum {
    kPlayerCount = 4,
    kActionCount = 39,
    kActionsPerPublishBatch = 3,
    kActionPublishBatches = kActionCount / kActionsPerPublishBatch,
    kCursorAppearanceCount = 6,
    kRumbleInactive = -1,
    kMotorOff = 0,
    kMotorOn = 1,
    kCursorOnly = 0,
    kCursorWithLabel = 1,
    kCursorWithPlayerNumber = 2,
    kCursorActiveAppearance = 2,
    kCursorClickAppearance = 3,
    kCursorClickFrames = 3,
    kCursorAppearance05 = 5, // Distinct number-label offset; further appearance meaning unverified.
    kCursorVertexFormat = 0x11F005,
    kCursorTriangleStrip = 0x98
};
enum CursorQuadLayout {
    kCursorQuadVertexCount = 4,
    kCursorVertexBytes = 0x18,
    kCursorTrailPointCapacity = 0x18,
    kCursorTrailMode = 2
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
// vectorExceptionTypeStrings
extern const char vectorExceptionTypeStrings[];
}
}

extern "C" {

// Return the selected controller channel, without resolving a device.
int InputManager_GetSelectedPlayer(void* input) {
    return AsGameInputManager(input)->selectedPlayer;
}

// Add one cursor-drawing suspension; each request needs a matching resume.
void InputManager_SuspendCursorDrawing(void* input) {
    AsGameInputManager(input)->cursorDrawingSuspendCount = AsGameInputManager(input)->cursorDrawingSuspendCount + 1;
}

// Return the current word-vector iterator pointer.
void* CursorLayoutIterator_GetPointer(void* p) {
    return ((WordIterator*)p)->current;
}

// Advance a word-vector iterator by one 32-bit element.
void CursorLayoutIterator_Advance(void* p) {
    ((WordIterator*)p)->current++;
}

// Return the current pointer-vector iterator pointer.
void* CursorTrailIterator_GetPointer(void* p) {
    return ((WordIterator*)p)->current;
}

// Advance a pointer-vector iterator by one Wii pointer.
void CursorTrailIterator_Advance(void* p) {
    ((WordIterator*)p)->current++;
}

// Initialize an empty cursor-layout pointer vector.
void CursorLayoutVector_Init(void* p) {
    ((PointerVector*)p)->data = 0;
    ((PointerVector*)p)->size = 0;
    ((PointerVector*)p)->capacity = 0;
}

// Initialize an empty cursor-trail pointer vector.
void CursorTrailVector_Init(void* p) {
    ((PointerVector*)p)->data = 0;
    ((PointerVector*)p)->size = 0;
    ((PointerVector*)p)->capacity = 0;
}

// Store per-controller cursor label mode and player-color selection.
// input required; player 0..3; labelMode 0..2; number selection playerColor 0..3.
// enabled chooses this option record instead of global policy, not motor state.
void InputManager_SetCursorDrawOptions(void* input, int enabled, int labelMode, int playerColor, int player) {
    CursorDrawOptions* options = &((CursorResources*)input)->drawOptions[player];
    options->enabled = enabled;
    options->labelMode = labelMode;
    options->playerColor = playerColor;
}

// Remove one drawing suspension, saturating at zero to avoid underflow.
void InputManager_ResumeCursorDrawing(void* input) {
    int suspendCount = AsGameInputManager(input)->cursorDrawingSuspendCount;
    if (suspendCount <= 0)
        return;
    AsGameInputManager(input)->cursorDrawingSuspendCount = suspendCount - 1;
}

// Provide a screen XY override; drawing consumes and clears the override flag.
void InputManager_SetPointerPositionOverride(void* input, void* positionXY) {
    AsGameInputManager(input)->pointerOverrideEnabled = 1;
    AsGameInputManager(input)->pointerOverrideX = ((float*)positionXY)[0];
    AsGameInputManager(input)->pointerOverrideY = ((float*)positionXY)[1];
}

// Raise the requested appearance priority for one controller cursor.
// appearance must be 0..5 and player 0..3; a lower/equal request leaves state
// unchanged. DrawPlayerCursor consumes/reset appearance after rendering.
void InputManager_RaiseCursorAppearance(void* input, int appearance, int player) {
    char* base = (char*)input + player * 4;
    int currentAppearance = AsGameInputManager(base)->cursorAppearance[0];
    if (currentAppearance >= appearance)
        return;
    AsGameInputManager(base)->cursorAppearance[0] = appearance;
}

extern CMouseCursor* g_mouseCursor;

// Combine this frame's pointer-input requests; Update clears them before callers request input again.
void InputManager_RequestPointerInput(void* unused, int requested) {
    void* cursor = g_mouseCursor;
    unsigned char enabled = ((CMouseCursor*)cursor)->pointerInputEnabled;
    int combinedRequests = requested | enabled;
    CMouseCursor::SetPointerInputEnabled(cursor, (combinedRequests != 0));
}

// Set a cursor RGB override; the original API copies the third color argument into both green and blue.
void InputManager_SetCursorColorOverride(void* input, int enabled, int red, int greenAndBlue, int unused, int player) {
    CursorColorOverride* color = &((CursorResources*)input)->colorOverrides[player];
    color->enabled = enabled;
    color->red = (unsigned char)red;
    color->green = (unsigned char)greenAndBlue;
    color->blue = (unsigned char)greenAndBlue;
}

// Compare the stored positions of two cursor-layout vector iterators.
int CursorLayoutIterator_NotEqual(int* a, int* b) {
    return *a != *b;
}

// Compare the stored positions of two cursor-trail vector iterators.
int CursorTrailIterator_NotEqual(int* a, int* b) {
    return *a != *b;
}

extern void* gtl_InputEx_GetScreenPointer(int x);

// Return the selected controller screen input unless lookup is disabled.
void* InputManager_GetSelectedPad(void* input) {
    if (AsGameInputManager(input)->selectedPadLookupDisabled != 0)
        return 0;
    return gtl_InputEx_GetScreenPointer(AsGameInputManager(input)->selectedPlayer);
}

// Select a controller channel, rejecting indices outside the four supported channels.
int InputManager_SetSelectedPlayer(void* input, int player) {
    if (player >= kPlayerCount || player < 0)
        return 0;
    AsGameInputManager(input)->selectedPlayer = player;
    return 1;
}

extern "C" int Layout_GetCursorPosition(void* p, void* out);
extern "C" int Layout_GetCursorDepth(void* p, void* out);
extern "C" int PadManager_SetMotorEnabled(void* p, int flag);
extern const float kFloat_0_BalloonInput;
extern "C" void InputManager_UpdateCursorColor(void* self, int idx);
extern "C" void InputManager_DrawPlayerCursor(void* self, int idx);
extern "C" void InputManager_DrawPlayerCursorWithOptions(void* self, void* out, int idx);
extern "C" void TrailRibbon_SetStyle(void* p, void* color);
// Advance the cursor trail after supplying the current pointer position.
extern "C" void TrailRibbon_Update(void* p);
extern "C" void TrailRibbon_SetPointerPosition(void* p, void* out);
extern "C" void TrailRibbon_Build(void* p);

// Read layout depth for a controller, using the selected controller for a negative index.
// depthOut is required writable float storage; no layout returns 0 WITHOUT
// touching it. Otherwise the layout supplies the status/value; positive player <4.
int InputManager_GetCursorDepth(void* input, void* depthOut, int player) {
    if (player < 0)
        player = AsGameInputManager(input)->selectedPlayer;
    void** layouts = AsGameInputManager(input)->cursorLayouts;
    void* layout = layouts[player];
    if (layout != 0)
        return Layout_GetCursorDepth(layout, depthOut);
    return 0;
}

// Read override or layout XY, using zero XY when no layout exists; report whether a position was available.
// positionXY requires float[2]; input must be live. The override is borrowed
// state and is NOT consumed by this query. Layout status is returned unchanged.
int InputManager_GetPointerPosition(void* input, float* positionXY, int player) {
    if (player < 0)
        player = AsGameInputManager(input)->selectedPlayer;
    if (AsGameInputManager(input)->pointerOverrideEnabled != 0) {
        float y;
        float x;
        x = (*(volatile float*)&AsGameInputManager(input)->pointerOverrideX);
        y = (*(volatile float*)&AsGameInputManager(input)->pointerOverrideY);
        positionXY[0] = x;
        positionXY[1] = y;
        return 1;
    }
    void** layouts = AsGameInputManager(input)->cursorLayouts;
    void* layout = layouts[player];
    if (0 != layout)
        return Layout_GetCursorPosition(layout, positionXY);
    float zero = kFloat_0_BalloonInput;
    positionXY[1] = zero;
    positionXY[0] = zero;
    return 0;
}

extern "C" void appFreeUni__FPv(void* p);
extern "C" void __dl__FPv(void* p);
extern "C" void __dla__FPv(void* p);
extern "C" void* appMallocDummy__FUl(unsigned int size);
extern "C" void* __nwa__FUl(unsigned int size);
extern "C" void __dt__Q23std12length_errorFv(void* p);
extern "C" void __throw(void* a, void* b, void* c) __attribute__((noreturn));
extern "C" unsigned int strlen(const char* s);
extern "C" char* strcpy(char* dst, const char* src);
extern "C" void* memset(void* dst, int value, unsigned int size);
extern "C" void* memcpy(void* dst, const void* src, unsigned int size);
extern "C" void CursorLayoutVector_Resize(void* self, unsigned int size);

void CursorLayoutVector_Resize(void* self, unsigned int size);
}

extern const char vectorExceptionTypeStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern "C" void* __nw__FUl(unsigned int size);

extern "C" void __dt__Q23std9exceptionFv(void* p, int flag);

struct V71Allocation {
    char* pointer;
    char* allocator;
    // Guard exception-message storage until the shared control block owns it.
    V71Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete the exception message unless ownership was transferred.
    ~V71Allocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct V71ControlBase {
    void* table;
    unsigned int strong, weak;
    // Initialize the shared exception-message strong/weak reference counts.
    V71ControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct V71Control : V71ControlBase {
    char* pointer;
    // Retain the message pointer in its shared control representation.
    V71Control(char* p) {
        pointer = p;
    }
};
struct V71ErrorBase {
    void* table;
    // Initialize the compatible standard logic-error base.
    V71ErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Run the standard exception-base destructor without freeing this storage.
    ~V71ErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : V71ErrorBase {
    char* text;
    void* control;
    // Copy the error message and transfer it to the shared character representation.
    length_error(const char* message) {
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            V71Allocation allocation(p, &allocator.value);
            control = new V71Control(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

// Construct and throw the CodeWarrior-compatible vector length exception.
static inline void v71_throw() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct("vector length error");
        exception_runtime::__throw(exception_runtime::vectorExceptionTypeStrings + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

struct V71Buf {
    unsigned int* p;
    unsigned int begin;
    unsigned int cap;
    unsigned int* alloc;
    unsigned int end;
    // Create an empty guard associated with the vector capacity word.
    V71Buf(unsigned int* a) : p(0), begin(0), cap(0), alloc(a), end(0) {}
    // Free the allocation left by a successful swap or a failed growth operation.
    ~V71Buf() {
        begin = 0;
        if (p) {
            begin = 0;
            appFreeUni__FPv(p);
        }
    }
};

// Return the larger vector-growth requirement by reference.
static inline const unsigned int& v71_max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Subtract vector counts using unsigned arithmetic.
static inline unsigned int v71_sub(unsigned int a, unsigned int b) {
    return a - b;
}

extern "C" {
#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Resize the cursor-layout pointer array; does not construct/delete layouts.
// Both Resize APIs require a valid {data,size,capacity}, size <= capacity <=
// kVectorMaxElements, and requested size <= kVectorMaxElements. New slots are null;
// shrink discards pointer words logically, so the caller must destroy payloads first.
// Length overflow throws length_error before publication. appMallocDummy__FUl
// (memory_heap_init_and_alloc.cpp) can return null; these helpers do not test it before memset/memcpy:
// allocation failure is NOT a recoverable false return or a guaranteed exception.
// No owner fields change before the commit; there is no rollback after it begins.
// V71Buf::alloc borrows &capacity as the stateless allocator token, not an arena.
// Its begin/end/cap fields and the zeroed moved-from words reproduce MSL's
// allocate/append/move/swap cleanup: the guard frees the old array after commit.
// These local optimizer blocks keep those loads, copy order, and cleanup lifetimes
// visible to MWCC; use_lmw_stmw selects the original multi-register save shape.
void CursorLayoutVector_Resize(void* self, unsigned int size) {
    PointerVector* vec = (PointerVector*)self;
    unsigned int count;
    unsigned int candidate1;
    unsigned int candidate2;
    unsigned int needed;
    unsigned int oldSize = vec->size;
    if (size > oldSize) {
        count = size - oldSize;
        // 0x8007102C: The capacity tiers share the growth preflight exit.
        do {
            if (count > vec->capacity || oldSize > vec->capacity - count) {
                if (vec->size + count - vec->capacity > kVectorMaxElements - vec->capacity)
                    v71_throw();
                unsigned int cp = vec->capacity;
                if (cp >= kVectorSmallTier) {
                    if (cp >= kVectorLargeTier)
                        break;
                    break;
                } else {
                    break;
                }
            } else {
                memset(vec->data + oldSize, 0, count * 4);
                vec->size += count;
                return;
            }
        } while (0);

        // Reserve: compute growth, allocate, and zero the new logical tail.
        needed = vec->size + count - vec->capacity;
        V71Buf buf(&vec->capacity);
        unsigned int newCapacity = vec->capacity;
        if (needed > kVectorMaxElements - newCapacity) {
            v71_throw();
        }
        if (newCapacity < kVectorSmallTier) {
            candidate1 = ((newCapacity + 1) * 3) / 5;
            newCapacity += v71_max(candidate1, needed);
        } else if (newCapacity < kVectorLargeTier) {
            candidate2 = (newCapacity + 1) / 2;
            newCapacity += v71_max(candidate2, needed);
        } else {
            newCapacity = kVectorMaxElements;
        }
        if (newCapacity > kVectorMaxElements)
            v71_throw();
        buf.p = (unsigned int*)appMallocDummy__FUl(newCapacity * 4);
        buf.cap = newCapacity;
        buf.end = vec->size;
        memset(buf.p + buf.end + buf.begin, 0, count * 4);
        // Move old pointer words to the front and clear their previous storage.
        unsigned int* last = vec->data + vec->size;
        int n = ((char*)last - (char*)vec->data) / 4;
        unsigned int* data = vec->data;
        int nbytes = n * 4;
        buf.begin += count;
        buf.end -= n;
        memcpy(buf.p + buf.end, data, nbytes);
        memset(data, 0, nbytes);
        buf.begin += n;
        // Publish the new buffer; the guard now owns the old allocation.
        unsigned int* t = vec->data;
        unsigned int nb = buf.begin;
        unsigned int c2 = vec->capacity;
        unsigned int* np = buf.p;
        vec->size = 0;
        buf.p = t;
        vec->capacity = buf.cap;
        buf.cap = c2;
        vec->data = np;
        vec->size = nb;
        // Shrink changes the logical range only; payload lifetime stays with the manager.
    } else if (size < oldSize) {
        vec->size -= oldSize - size;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
// Resize the cursor-trail pointer array under the same preconditions, failure
// publication, allocator-token and pointee-lifetime contract as LayoutVector_Resize.
// This block disables propagation/common-subexpression folding to retain the
// capacity reload and reserve/swap shape; it does not add synchronization.
void CursorTrailVector_Resize(void* self, unsigned int size) {
    PointerVector* vec = (PointerVector*)self;
    unsigned int count;
    unsigned int candidate1;
    unsigned int candidate2;
    unsigned int needed;
    unsigned int oldSize = vec->size;
    if (size > oldSize) {
        count = size - oldSize;
        // 0x80071510: The capacity tiers share the growth preflight exit.
        do {
            if (count > vec->capacity || oldSize > vec->capacity - count) {
                if (vec->size + count - vec->capacity > kVectorMaxElements - vec->capacity)
                    v71_throw();
                unsigned int cp = vec->capacity;
                if (cp >= kVectorSmallTier) {
                    if (cp >= kVectorLargeTier)
                        break;
                    break;
                } else {
                    break;
                }
            } else {
                memset(vec->data + oldSize, 0, count * 4);
                vec->size += count;
                return;
            }
        } while (0);

        // Reserve: compute growth, allocate, and zero the new logical tail.
        needed = vec->size + count - vec->capacity;
        V71Buf buf(&vec->capacity);
        unsigned int newCapacity = vec->capacity;
        if (needed > kVectorMaxElements - newCapacity) {
            v71_throw();
        }
        if (newCapacity < kVectorSmallTier) {
            candidate1 = ((newCapacity + 1) * 3) / 5;
            newCapacity += v71_max(candidate1, needed);
        } else if (newCapacity < kVectorLargeTier) {
            candidate2 = (newCapacity + 1) / 2;
            newCapacity += v71_max(candidate2, needed);
        } else {
            newCapacity = kVectorMaxElements;
        }
        if (newCapacity > kVectorMaxElements)
            v71_throw();
        buf.p = (unsigned int*)appMallocDummy__FUl(newCapacity * 4);
        buf.cap = newCapacity;
        buf.end = vec->size;
        memset(buf.p + buf.end + buf.begin, 0, count * 4);
        // Move old pointer words to the front and clear their previous storage.
        unsigned int* last = vec->data + vec->size;
        int n = ((char*)last - (char*)vec->data) / 4;
        unsigned int* data = vec->data;
        int nbytes = n * 4;
        buf.begin += count;
        buf.end -= n;
        memcpy(buf.p + buf.end, data, nbytes);
        memset(data, 0, nbytes);
        buf.begin += n;
        // Publish the new buffer; the guard now owns the old allocation.
        unsigned int* t = vec->data;
        unsigned int nb = buf.begin;
        unsigned int c2 = vec->capacity;
        unsigned int* np = buf.p;
        vec->size = 0;
        buf.p = t;
        vec->capacity = buf.cap;
        buf.cap = c2;
        vec->data = np;
        vec->size = nb;
        // Shrink changes the logical range only; payload lifetime stays with the manager.
    } else if (size < oldSize) {
        vec->size -= oldSize - size;
    }
}
#pragma pop

// Stop every controller motor and discard all scheduled rumble durations.
int InputManager_StopAllRumble(void* input) {
    char* channelCursor = (char*)input;
    for (int player = 0; player < kPlayerCount; player++) {
        void* screenInput = gtl_InputEx_GetScreenPointer(player);
        if (screenInput != 0) {
            void* motorDevice = ((ScreenInput*)screenInput)->motorDevice;
            PadManager_SetMotorEnabled(motorDevice, kMotorOff);
        }
        AsGameInputManager(channelCursor)->rumbleFramesRemaining[0] = kRumbleInactive;
        channelCursor += 4;
    }
    return 1;
}

// Release only the cursor-layout pointer array; delete the vector wrapper when flag > 0.
void* CursorLayoutVector_dtor(void* self, int flag) {
    return std::__vector_deleting_dtor((CursorPointerVector*)self, flag);
}

// Destructor of the cursor-trail vector: same shape as CursorLayoutVector_dtor (frees the pointer array only).
void* CursorTrailVector_dtor(void* self, int flag) {
    return std::__vector_deleting_dtor((CursorPointerVector*)self, flag);
}

// Stop every motor while retaining scheduled durations for a later resume.
int InputManager_PauseRumble(void* input) {
    for (int player = 0; player < kPlayerCount; player++) {
        void* screenInput = gtl_InputEx_GetScreenPointer(player);
        if (screenInput != 0) {
            void* motorDevice = ((ScreenInput*)screenInput)->motorDevice;
            PadManager_SetMotorEnabled(motorDevice, kMotorOff);
        }
    }
    AsGameInputManager(input)->rumblePaused = 1;
    return 1;
}

// Restart motors with a pending duration and allow the rumble timers to advance again.
int InputManager_ResumeRumble(void* input) {
    char* channelCursor = (char*)input;
    for (int player = 0; player < kPlayerCount; player++) {
        if (AsGameInputManager(channelCursor)->rumbleFramesRemaining[0] >= 0) {
            void* screenInput = gtl_InputEx_GetScreenPointer(player);
            if (screenInput != 0) {
                void* motorDevice = ((ScreenInput*)screenInput)->motorDevice;
                PadManager_SetMotorEnabled(motorDevice, kMotorOn);
            }
        }
        channelCursor += 4;
    }
    AsGameInputManager(input)->rumblePaused = 0;
    return 1;
}

// Start the selected controller motor for a positive frame count unless rumble is paused.
int InputManager_StartRumble(void* input, int frames) {
    if (frames <= 0)
        return 0;
    if (AsGameInputManager(input)->rumblePaused != 0)
        return 0;
    void* screenInput = gtl_InputEx_GetScreenPointer(AsGameInputManager(input)->selectedPlayer);
    if (screenInput == 0)
        return 0;
    void* motorDevice = ((ScreenInput*)screenInput)->motorDevice;
    PadManager_SetMotorEnabled(motorDevice, kMotorOn);
    AsGameInputManager(input)->rumbleFramesRemaining[AsGameInputManager(input)->selectedPlayer] = frames;
    return 1;
}

// Stable game action protocol; several IDs intentionally share a control.
// Physical lookup is category -> CScreenPointer::buttons[category] (slot 3,
// gtl_input_ex_devices_part2.cpp) -> flags produced by CScreenPointer_vfunc_6/UpdateButtons
// (gtl_input_ex_devices_part1.cpp) from raw sample byte +0x30+category. The stock report descriptor
// s_padAndScreenPointerInitParameters has 32 consecutive button entries at +30..+4F; its first 16
// feed CScreenPointer. InputDriver_BuildStateReport (gtl_pad_drivers_gc_classic_freestyle.cpp) matches each
// to the next unused button item. CRvlPointingDeviceDriver_BuildStateReports
// (input_device_drivers.cpp) supplies those items in Up,Down,Left,Right,+,-,Home,A,B,1,2 order.
// Descriptor ordering is essential: there is no hardware bitmask at this API.
// The descriptor routing and s_screenPointerLogicalButtonItemKeys item keys (+30..+3F) were checked
// against the stock tables; changing provider/descriptor can change this map.
// Stock Remote lookup (category : control <- game actions):
//  0:Up <- 0; 1:Down <- 1,13; 2:Left <- 2; 3:Right <- 3;
//  4:+ <- 9,17,27,28; 5:- <- 10,18,37,38; 6:Home <- 19;
//  7:A <- 4,5,11,12,15,21,24,26,32;
//  8:B <- 6,8,14,16,22,23,25,30,35;
//  9:1 <- 20,31,33,34,36; 10:2 <- 7,29.
// Other devices have their own item ordering. These names describe the stock
// Remote route used by these queries, not generic Classic/GameCube support.
enum InputActionId {
    kActionId00 = 0,
    kActionId01 = 1,
    kActionId02 = 2,
    kActionId03 = 3,
    kActionId04 = 4,
    kActionId05 = 5,
    kActionId06 = 6,
    kActionId07 = 7,
    kActionId08 = 8,
    kActionId09 = 9,
    kActionId10 = 10,
    kActionId11 = 11,
    kActionId12 = 12,
    kActionId13 = 13,
    kActionId14 = 14,
    kActionId15 = 15,
    kActionId16 = 16,
    kActionId17 = 17,
    kActionId18 = 18,
    kActionId19 = 19,
    kActionId20 = 20,
    kActionId21 = 21,
    kActionId22 = 22,
    kActionId23 = 23,
    kActionId24 = 24,
    kActionId25 = 25,
    kActionId26 = 26,
    kActionId27 = 27,
    kActionId28 = 28,
    kActionId29 = 29,
    kActionId30 = 30,
    kActionId31 = 31,
    kActionId32 = 32,
    kActionId33 = 33,
    kActionId34 = 34,
    kActionId35 = 35,
    kActionId36 = 36,
    kActionId37 = 37,
    kActionId38 = 38,
};
enum InputButtonCategory {
    kRemoteCategoryUp = 0,
    kRemoteCategoryDown = 1,
    kRemoteCategoryLeft = 2,
    kRemoteCategoryRight = 3,
    kRemoteCategoryPlus = 4,
    kRemoteCategoryMinus = 5,
    kRemoteCategoryHome = 6,
    kRemoteCategoryA = 7,
    kRemoteCategoryB = 8,
    kRemoteCategoryOne = 9,
    kRemoteCategoryTwo = 10,
};
enum MenuAdvanceAction {
    kMenuAdvanceAction04 = kActionId04,
    kMenuAdvanceAction12 = kActionId12,
    kMenuAdvanceAction06 = kActionId06,
    kMenuAdvanceAction27 = kActionId27,
    kMenuAdvanceAction07 = kActionId07,
    kMenuAdvanceAction08 = kActionId08
};

// Map 39 game action IDs onto the controller button categories; unrelated actions can share a category.
// unused is an ABI receiver and is ignored. Return is a category INDEX, not a
// button bit mask. Out-of-domain IDs map to category 0 only here; query APIs
// index action arrays first and therefore require 0..38. HOME action 19 -> 6;
// menu confirm 4 -> 7 and cancel 6 -> 8. Do not infer one physical button per
// action: see the complete stock Remote lookup above; provider report ordering
// and descriptor entries decide what a category reads.
int Input_MapActionToCategory(void* unused, unsigned int action) {
    switch (action) {
    case kActionId00:
        return kRemoteCategoryUp;
    case kActionId01:
    case kActionId13:
        return kRemoteCategoryDown;
    case kActionId02:
        return kRemoteCategoryLeft;
    case kActionId03:
        return kRemoteCategoryRight;
    case kActionId04:
    case kActionId05:
    case kActionId11:
    case kActionId12:
    case kActionId15:
    case kActionId21:
    case kActionId24:
    case kActionId26:
    case kActionId32:
        return kRemoteCategoryA;
    case kActionId08:
    case kActionId14:
    case kActionId16:
    case kActionId22:
    case kActionId23:
    case kActionId25:
    case kActionId30:
    case kActionId35:
        return kRemoteCategoryB;
    case kActionId06:
        return kRemoteCategoryB;
    case kActionId10:
    case kActionId18:
    case kActionId37:
    case kActionId38:
        return kRemoteCategoryMinus;
    case kActionId07:
        return kRemoteCategoryTwo;
    case kActionId31:
    case kActionId33:
    case kActionId34:
        return kRemoteCategoryOne;
    case kActionId19:
        return kRemoteCategoryHome;
    case kActionId17:
        return kRemoteCategoryPlus;
    case kActionId20:
        return kRemoteCategoryOne;
    case kActionId09:
    case kActionId27:
        return kRemoteCategoryPlus;
    case kActionId28:
        return kRemoteCategoryPlus;
    case kActionId29:
        return kRemoteCategoryTwo;
    case kActionId36:
        return kRemoteCategoryOne;
    }
    return kRemoteCategoryUp; // Preserve the original default category zero.
}

// Optional action veto hook; the current implementation at 0x802D5514 returns zero.
// Keep the call: other builds/mods can supply a veto even though this build always allows input.
extern "C" int GameHook_ShouldBlockInput(void* p);
extern "C" int GetButtonPressed__Q23gtl18CInputExDeviceBaseFv(void* p, int mask);
extern "C" int GetButtonDown__Q23gtl18CInputExDeviceBaseFv(void* p, int mask);
extern "C" void Layout_UpdateCursorFromPlayer(void* p);
extern CSharedDoc* g_sharedDoc;

// Return a synthetic press first; otherwise honor both action blocks and query the controller press state.
// mgr required, action 0..38, player <0 selects the current channel else 0..3.
// Hook veto wins over synthetic input; synthetic input then bypasses BOTH block
// arrays. Missing physical device returns 0. No press is consumed by this query.
int Input_IsActionPressed(void* mgr, int action, int player) {
    if (GameHook_ShouldBlockInput(mgr)) {
        return 0;
    }
    GameInputManager* input = AsGameInputManager(mgr);
    if (input->actionPress[action] != 0) {
        return 1;
    }
    if (input->actionBlocked[action] != 0) {
        return 0;
    }
    if (input->actionBlockedSecondary[action] != 0) {
        return 0;
    }
    if (player < 0) {
        player = AsGameInputManager(mgr)->selectedPlayer;
    }
    void* pad = gtl_InputEx_GetScreenPointer(player);
    if (pad == 0) {
        return 0;
    }
    int category = ((int (*)(void*, unsigned int, void*))Input_MapActionToCategory)(mgr, (unsigned int)action,
                                                                                    (void*)(long)player);
    return GetButtonPressed__Q23gtl18CInputExDeviceBaseFv(pad, category);
}

// Return a synthetic press first; otherwise honor both action blocks and query the held-button state.
// Same index/priority contract as Input_IsActionPressed, including synthetic
// bypass of blocks; native path queries held state rather than a press edge.
int Input_IsActionButtonDown(void* mgr, int action, int player) {
    if (GameHook_ShouldBlockInput(mgr)) {
        return 0;
    }
    GameInputManager* input = AsGameInputManager(mgr);
    if (input->actionPress[action] != 0) {
        return 1;
    }
    if (input->actionBlocked[action] != 0) {
        return 0;
    }
    if (input->actionBlockedSecondary[action] != 0) {
        return 0;
    }
    if (player < 0) {
        player = AsGameInputManager(mgr)->selectedPlayer;
    }
    void* pad = gtl_InputEx_GetScreenPointer(player);
    if (pad == 0) {
        return 0;
    }
    int category = ((int (*)(void*, unsigned int, void*))Input_MapActionToCategory)(mgr, (unsigned int)action,
                                                                                    (void*)(long)player);
    return GetButtonDown__Q23gtl18CInputExDeviceBaseFv(pad, category);
}

// Publish queued synthetic presses, advance rumble timers, choose cursor mode, and refresh layout pointer positions.
int InputManager_Update(void* mgr) {
    void* mouseCursor = g_mouseCursor;
    if (mouseCursor != 0) {
        CMouseCursor::SetPointerInputEnabled(mouseCursor, 0);
    }
    {
        // Publish 39 pending synthetic presses in batches of three and consume each latch.
        char* actionCursor = (char*)mgr;
        int unused = 0;
        for (int i = 0; i < kActionPublishBatches; i++) {
            AsGameInputManager(actionCursor)->actionPress[0] = 0;
            if (AsGameInputManager(actionCursor)->pendingActionPress[0] != 0) {
                AsGameInputManager(actionCursor)->actionPress[0] = 1;
                AsGameInputManager(actionCursor)->pendingActionPress[0] = 0;
            }
            AsGameInputManager(actionCursor)->actionPress[1] = 0;
            if (AsGameInputManager(actionCursor)->pendingActionPress[1] != 0) {
                AsGameInputManager(actionCursor)->actionPress[1] = 1;
                AsGameInputManager(actionCursor)->pendingActionPress[1] = 0;
            }
            AsGameInputManager(actionCursor)->actionPress[2] = 0;
            if (AsGameInputManager(actionCursor)->pendingActionPress[2] != 0) {
                AsGameInputManager(actionCursor)->actionPress[2] = 1;
                AsGameInputManager(actionCursor)->pendingActionPress[2] = 0;
            }
            actionCursor += 0xc;
            unused += 2;
        }
        (void)unused;
    }
    {
        // Decrement active countdowns; retry motor-off until the device acknowledges it.
        char* rumbleCursor = (char*)mgr;
        int i = 0;
        int inactiveRumble = kRumbleInactive;
        while (i < kPlayerCount) {
            if (AsGameInputManager(mgr)->rumblePaused == 0) {
                int framesRemaining = AsGameInputManager(rumbleCursor)->rumbleFramesRemaining[0];
                if (framesRemaining >= 0) {
                    if (framesRemaining == 0) {
                        void* pad = gtl_InputEx_GetScreenPointer(i);
                        if (pad != 0) {
                            void* motorDevice = ((ScreenInput*)pad)->motorDevice;
                            if (PadManager_SetMotorEnabled(motorDevice, kMotorOff) != 0) {
                                AsGameInputManager(rumbleCursor)->rumbleFramesRemaining[0] = inactiveRumble;
                            }
                        }
                    } else {
                        AsGameInputManager(rumbleCursor)->rumbleFramesRemaining[0] = framesRemaining - 1;
                    }
                }
            }
            i++;
            rumbleCursor += 4;
        }
    }
    void* sharedDoc = g_sharedDoc;
    // 0x8006F060: Choose cursor mode before updating the cursor layouts.
    do {
        if (sharedDoc != 0) {
            void* golfDoc = (void*)((CSharedDoc*)sharedDoc)->golfDoc;
            if (golfDoc != 0) {
                unsigned char playerCount = AsCGolfDoc(golfDoc)->playerCount;
                if (playerCount != 0) {
                    AsGameInputManager(mgr)->cursorMode = kCursorWithPlayerNumber;
                    break;
                }
            }
            if (AsGameInputManager(mgr)->cursorModeRequestCount == 0) {
                AsGameInputManager(mgr)->cursorMode = kCursorOnly;
            } else {
                AsGameInputManager(mgr)->cursorMode = kCursorWithLabel;
            }
        } else {
            AsGameInputManager(mgr)->cursorMode = kCursorOnly;
        }
    } while (0);

    {
        void** layoutIterator = AsGameInputManager(mgr)->cursorLayouts;
        while (layoutIterator != AsGameInputManager(mgr)->cursorLayouts + AsGameInputManager(mgr)->cursorLayoutCount) {
            Layout_UpdateCursorFromPlayer(*layoutIterator);
            layoutIterator++;
        }
    }
    {
        // Existing empty walk over trail slots; this does not poll physical input.
        char* trailSlot = (char*)(*(void**)&AsGameInputManager(mgr)->cursorTrails);
        char* trailEnd = trailSlot + AsGameInputManager(mgr)->cursorTrailCount * 4;
        while (trailSlot != trailEnd) {
            trailSlot += 4;
        }
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Delete cursor layouts and trails, release their owned arrays, and optionally free the manager.
// Existing behavior: this destructor omits GraphicsResource_Release for all 14
// decoded texture/palette slots. Their initial owned references therefore leak;
// a bound render stage's extra reference does not discharge the manager's one.
// They are graphics handles, never raw allocations to pass to delete/appFree.
void* InputManager_Dtor(void* self, int deleting) {
    if (self != 0) {
        void** layoutIterator = AsGameInputManager(self)->cursorLayouts;
        while (layoutIterator !=
               AsGameInputManager(self)->cursorLayouts + AsGameInputManager(self)->cursorLayoutCount) {
            if (*layoutIterator != 0) {
                __dl__FPv(*layoutIterator);
                *layoutIterator = 0;
            }
            layoutIterator++;
        }
        // Destroy trail payloads, including both owned vertex-array families, before vector storage.
        void** trailIterator = AsGameInputManager(self)->cursorTrails;
        while (trailIterator != AsGameInputManager(self)->cursorTrails + AsGameInputManager(self)->cursorTrailCount) {
            void* trail = *trailIterator;
            if (trail != 0) {
                if (trail != 0) {
                    void* samples = ((CursorTrailStorage*)trail)->samples;
                    if (samples != 0) {
                        __dla__FPv(samples);
                        ((CursorTrailStorage*)trail)->samples = 0;
                    }
                    for (int arrayIndex = 0; arrayIndex < ((CursorTrailStorage*)trail)->arrayCount; arrayIndex++) {
                        void** arrays = ((CursorTrailStorage*)trail)->sourceVertexArrays;
                        void* allocation = arrays[arrayIndex];
                        if (allocation != 0) {
                            __dla__FPv(allocation);
                            arrays = ((CursorTrailStorage*)trail)->sourceVertexArrays;
                            arrays[arrayIndex] = 0;
                        }
                        arrays = ((CursorTrailStorage*)trail)->animatedVertexArrays;
                        allocation = arrays[arrayIndex];
                        if (allocation != 0) {
                            __dla__FPv(allocation);
                            arrays = ((CursorTrailStorage*)trail)->animatedVertexArrays;
                            arrays[arrayIndex] = 0;
                        }
                    }
                    void** arrays = ((CursorTrailStorage*)trail)->sourceVertexArrays;
                    if (arrays != 0) {
                        __dla__FPv(arrays);
                        ((CursorTrailStorage*)trail)->sourceVertexArrays = 0;
                    }
                    arrays = ((CursorTrailStorage*)trail)->animatedVertexArrays;
                    if (arrays != 0) {
                        __dla__FPv(arrays);
                        ((CursorTrailStorage*)trail)->animatedVertexArrays = 0;
                    }
                    void* pointAngles = ((CursorTrailStorage*)trail)->pointAngles;
                    if (pointAngles != 0) {
                        __dla__FPv(pointAngles);
                        ((CursorTrailStorage*)trail)->pointAngles = 0;
                    }
                    __dl__FPv(trail);
                }
            }
            *trailIterator = 0;
            trailIterator++;
        }
        ((CursorResources*)self)->drawOptions[0].enabled = 0;
        ((CursorResources*)self)->drawOptions[1].enabled = 0;
        ((CursorResources*)self)->drawOptions[2].enabled = 0;
        ((CursorResources*)self)->drawOptions[3].enabled = 0;
        // Release the two pointer arrays after all payloads have been deleted.
        // Plain C: if (trails) { trailCount = 0; free(trails); } then the same for the layout array. The stacked
        // address-of tests (always true) come from inlined vector destructors; collapsing them changes codegen.

        if (&AsGameInputManager(self)->cursorTrails) {
            if (&AsGameInputManager(self)->cursorTrails) {
                if (&AsGameInputManager(self)->cursorTrails) {
                    if (&AsGameInputManager(self)->cursorTrails) {
                        if (&AsGameInputManager(self)->cursorTrails) {
                            if (AsGameInputManager(self)->cursorTrails != 0) {
                                AsGameInputManager(self)->cursorTrailCount = 0;
                                appFreeUni__FPv(AsGameInputManager(self)->cursorTrails);
                            }
                        }
                    }
                }
            }
        }
        if (&AsGameInputManager(self)->cursorLayouts) {
            if (&AsGameInputManager(self)->cursorLayouts) {
                if (&AsGameInputManager(self)->cursorLayouts) {
                    if (&AsGameInputManager(self)->cursorLayouts) {
                        if (&AsGameInputManager(self)->cursorLayouts) {
                            if (AsGameInputManager(self)->cursorLayouts != 0) {
                                AsGameInputManager(self)->cursorLayoutCount = 0;
                                appFreeUni__FPv(AsGameInputManager(self)->cursorLayouts);
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

void* GolfPlayer_GetEntryByIndex(void* p, int idx);

extern "C" void* gtl_InputEx_GetScreenPointer(int player);
extern "C" void Engine_SetFlexibleVertexFormat(void* rect);
extern "C" void Engine_SetTextureStageResource(int a, void* p);
extern "C" void Engine_SetTexturePalette(int a, void* p);
extern "C" void* GX_GetPrimitiveCount(int a, int b);
extern "C" void DrawPrimitiveUP(int prim, void* buffer, void* vertices, int stride);
// kCursorUvExtent = 0.625; kCursorSizePixels = 40; kCursorHalfSizePixels = 20; kCursorLabelWidthPixels = 50
extern "C" float kCursorUvExtent, kCursorSizePixels, kCursorHalfSizePixels, kCursorLabelWidthPixels;
// kCursorLabelUvWidth = 0.78125; kCursorLabelUvHeight = 1; kCursorCenteringFactor = 0.5; kCursorLabelVerticalOffset = -7
extern "C" float kCursorLabelUvWidth, kCursorLabelUvHeight, kCursorCenteringFactor, kCursorLabelVerticalOffset;
// kCursorLabelHeightPixels = 16; kCursorNumberWidthPixels = 28; kCursorNumberSpecialOffset = 2; kCursorNumberDefaultOffset = 7
extern "C" float kCursorLabelHeightPixels, kCursorNumberWidthPixels, kCursorNumberSpecialOffset,
    kCursorNumberDefaultOffset;
// kCursorNumberUvExtent = 0.875; kCursorNumberHeightPixels = 14
extern "C" float kCursorNumberUvExtent, kCursorNumberHeightPixels;
// cursorResourceTemplates
extern "C" unsigned char cursorResourceTemplates[];

// Common draw record for BOTH paths: native Wii floats/bytes, 4-byte aligned.
// Vertex +00 XYZ screen pixels/depth, +0C RGBA bytes, +10 normalized UV; stride
// 0x18, quad 0x60. Triangle-strip order is top-left, top-right, bottom-left,
// bottom-right. Fill routines preserve template Z/alpha; only RGB is overridden.
// Rect {x,y,right,bottom} is pixel edges, {u,v,ur,vb} is left/top/right/bottom UV.
struct CursorDrawVertex {
    float x, y, z;
    unsigned char r, g, b, a;
    float u, v;
};
struct CursorDrawBatch {
    CursorDrawVertex v[4];
    // Copy the complete quad template, preserving depth and alpha.
    CursorDrawBatch(const CursorDrawBatch& other) {
        *this = other;
    }
};
typedef CursorDrawBatch CursorBatch704A8;
struct CursorTemplates704A8 {
    char pad[0x248];
    CursorBatch704A8 cursor, label, number;
};
struct CursorDrawColor {
    unsigned char r, g, b, a;
};
typedef CursorDrawColor CursorColor704A8;
// This is the same manager tail as CursorResources and GameInputManager:
// +394/+3A4 label handles, +3B4 appearance[4], +3C4 label mode, +3C8 selected
// channel, +3CC click-draw counters. No separate alternate-path ownership.
struct CursorDrawState {
    char pad[0x394];
    void* labelTextures[4];
    void* labelPalettes[4];
    int appearance[4];
    int labelMode;
    int selectedPlayer;
    int tick[4];
};
typedef CursorDrawState CursorState704A8;
struct CursorDrawRect {
    float x, y, right, bottom, u, v, ur, vb;
};
typedef CursorDrawRect CursorRect704A8;
typedef char CursorDrawVertex_size[sizeof(CursorDrawVertex) == 0x18 ? 1 : -1];
typedef char CursorDrawBatch_size[sizeof(CursorDrawBatch) == 0x60 ? 1 : -1];
// Consume the pointer override and build a centered pixel rectangle with the cursor atlas UV extent.
inline void CursorInit704A8(CursorRect704A8& r, const float* pos, char* s) {
    float half, x, right, y, size, zero, uv;
    zero = *(volatile float*)&kFloat_0_BalloonInput;
    (*(volatile int*)&AsGameInputManager(s)->pointerOverrideEnabled) = 0;
    uv = *(volatile float*)&kCursorUvExtent;
    x = pos[0];
    size = kCursorSizePixels;
    y = pos[1];
    right = size + x;
    half = kCursorHalfSizePixels;
    size += y;
    r.v = zero;
    x -= half;
    right -= half;
    r.u = zero;
    y -= half;
    zero = size - half;
    r.x = x;
    r.right = right;
    r.y = y;
    r.bottom = zero;
    r.vb = uv;
    r.ur = uv;
}
// Write rectangle positions and UVs into the four cursor-template vertices.
inline void CursorFill704A8(CursorBatch704A8& b, CursorRect704A8& r) {
    b.v[0].x = r.x;
    b.v[0].y = r.y;
    b.v[0].u = r.u;
    b.v[0].v = r.v;
    b.v[1].x = r.right;
    b.v[1].y = r.y;
    b.v[1].u = r.ur;
    b.v[1].v = r.v;
    b.v[2].x = r.x;
    b.v[2].y = r.bottom;
    b.v[2].u = r.u;
    b.v[2].v = r.vb;
    b.v[3].x = r.right;
    b.v[3].y = r.bottom;
    b.v[3].u = r.ur;
    b.v[3].v = r.vb;
}
// Write positions, UVs, and RGB into a cursor template while preserving its alpha.
inline void CursorFillColor704A8(CursorBatch704A8& b, CursorRect704A8& r, const CursorColor704A8& c) {
    b.v[0].x = r.x;
    b.v[0].y = r.y;
    b.v[0].r = c.r;
    b.v[0].g = c.g;
    b.v[0].b = c.b;
    b.v[0].u = r.u;
    b.v[0].v = r.v;
    b.v[1].x = r.right;
    b.v[1].y = r.y;
    b.v[1].r = c.r;
    b.v[1].g = c.g;
    b.v[1].b = c.b;
    b.v[1].u = r.ur;
    b.v[1].v = r.v;
    b.v[2].x = r.x;
    b.v[2].y = r.bottom;
    b.v[2].r = c.r;
    b.v[2].g = c.g;
    b.v[2].b = c.b;
    b.v[2].u = r.u;
    b.v[2].v = r.vb;
    b.v[3].x = r.right;
    b.v[3].y = r.bottom;
    b.v[3].r = c.r;
    b.v[3].g = c.g;
    b.v[3].b = c.b;
    b.v[3].u = r.ur;
    b.v[3].v = r.vb;
}

// Draw a controller cursor and optional label/player number using caller-supplied draw options.
// self and options required; player must be 0..3 even though the position-only
// subpath can resolve negative indices. Missing device returns before consuming
// the override. options is copied by the dispatcher and never retained here.
// options must contain CursorDrawOptions; labelMode 0..2 and playerColor 0..3,
// even if the current mode does not draw a number. In number mode, playerColor
// directly selects the paired number texture/palette: the channel-to-golf-player
// search below does not alter it. Draw requires successfully decoded texture
// slots and their matching palette (which may be null for a nonpaletted image).
// LoadCursorResources ignores decoder status, so caller must establish that
// readiness. This API neither validates indices nor reports load failure.
void InputManager_DrawPlayerCursorWithOptions(void* self, void* options, int player) {
    CursorTemplates704A8* templates = (CursorTemplates704A8*)cursorResourceTemplates;
    char* inputBytes = (char*)self;

    CursorState704A8* state = (CursorState704A8*)self;
    if (!gtl_InputEx_GetScreenPointer(player))
        return;
    float positionXY[2] = {kFloat_0_BalloonInput, kFloat_0_BalloonInput};
    int layoutPlayer;
    if (0 > player)
        layoutPlayer = AsGameInputManager(inputBytes)->selectedPlayer;
    else
        layoutPlayer = player;
    if (AsGameInputManager(inputBytes)->pointerOverrideEnabled) {
        positionXY[0] = AsGameInputManager(inputBytes)->pointerOverrideX;
        positionXY[1] = AsGameInputManager(inputBytes)->pointerOverrideY;
    } else {
        void* item = (AsGameInputManager(inputBytes)->cursorLayouts)[layoutPlayer];
        if (item)
            Layout_GetCursorPosition(item, positionXY);
        else {
            positionXY[0] = positionXY[1] = kFloat_0_BalloonInput;
        }
    }
    CursorRect704A8 rect;
    CursorInit704A8(rect, positionXY, inputBytes);
    Engine_SetFlexibleVertexFormat((void*)kCursorVertexFormat);

    CursorColor704A8 color = *((CursorColor704A8*)AsGameInputManager(inputBytes)->cursorColors + player);
    CursorBatch704A8 cursorVertices = (*(CursorBatch704A8*)((char*)(&templates->cursor)));
    CursorFillColor704A8(cursorVertices, rect, color);

    if (state->appearance[player] == kCursorClickAppearance)
        state->tick[player] = 1;
    if (state->tick[player] && kCursorClickAppearance > state->appearance[player])
        state->appearance[player] = kCursorClickAppearance;
    Engine_SetTextureStageResource(0, ((CursorResources*)inputBytes)->cursorTextures[state->appearance[player]]);
    Engine_SetTexturePalette(0, ((CursorResources*)inputBytes)->cursorPalettes[state->appearance[player]]);
    DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                    &cursorVertices, kCursorVertexBytes);
    int tick = state->tick[player];
    if (tick && kCursorClickFrames > tick)
        state->tick[player] = tick + 1;
    else
        state->tick[player] = 0;

    // Label modes: 0 cursor only, 1 channel label, 2 player number from options.playerColor.
    switch ((((CursorDrawOptions*)options)->labelMode)) {
    case kCursorOnly:
        break;
    case kCursorWithLabel: {
        float width = rect.right - rect.x - kCursorLabelWidthPixels;
        rect.x = width * kCursorCenteringFactor + rect.x;
        rect.right = (kCursorLabelWidthPixels + rect.x);
        rect.y = kCursorLabelVerticalOffset + rect.bottom;
        rect.bottom = kCursorLabelHeightPixels + rect.y;
        rect.u = rect.v = kFloat_0_BalloonInput;
        rect.ur = kCursorLabelUvWidth;
        rect.vb = kCursorLabelUvHeight;

        CursorBatch704A8 labelVertices = (*(CursorBatch704A8*)((char*)(&templates->label)));
        CursorFill704A8(labelVertices, rect);

        Engine_SetTextureStageResource(0, state->labelTextures[player]);
        Engine_SetTexturePalette(0, state->labelPalettes[player]);
        DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                        &labelVertices, kCursorVertexBytes);
        break;
    }
    case kCursorWithPlayerNumber: {
        void* sharedDoc = g_sharedDoc;
        if ((void*)((CSharedDoc*)sharedDoc)->golfDoc) {
            float width = rect.right - rect.x - kCursorNumberWidthPixels;
            float delta = kFloat_0_BalloonInput;
            rect.x = width * kCursorCenteringFactor + rect.x;

            rect.right = kCursorNumberWidthPixels + rect.x;
            if (kCursorAppearance05 == state->appearance[player])
                delta -= kCursorNumberSpecialOffset;
            else
                delta -= kCursorNumberDefaultOffset;
            rect.y = rect.bottom + delta;
            float yValue = kCursorNumberHeightPixels + rect.y;
            rect.u = rect.v = kFloat_0_BalloonInput;
            rect.bottom = yValue;
            rect.ur = rect.vb = kCursorNumberUvExtent;

            CursorBatch704A8 numberVertices = (*(CursorBatch704A8*)((char*)(&templates->number)));
            CursorFill704A8(numberVertices, rect);

            if (AsGameInputManager(inputBytes)->cursorModeRequestCount) {
                int golfPlayer = 0;
                while (golfPlayer < AsCGolfDoc((void*)((CSharedDoc*)g_sharedDoc)->golfDoc)->playerCount) {
                    if (*(int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc,
                                                          (unsigned char)golfPlayer) == player)
                        break;
                    golfPlayer += 1;
                }
                Engine_SetTextureStageResource(
                    0, ((CursorResources*)inputBytes)->numberTextures[((CursorDrawOptions*)options)->playerColor]);
                Engine_SetTexturePalette(
                    0, ((CursorResources*)inputBytes)->numberPalettes[((CursorDrawOptions*)options)->playerColor]);
            } else {
                Engine_SetTextureStageResource(
                    0, ((CursorResources*)inputBytes)->numberTextures[((CursorDrawOptions*)options)->playerColor]);
                Engine_SetTexturePalette(
                    0, ((CursorResources*)inputBytes)->numberPalettes[((CursorDrawOptions*)options)->playerColor]);
            }
            DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                            &numberVertices, kCursorVertexBytes);
        }
    }
    }
    state->appearance[player] = 0;
}

// ABI-address aliases share the schema above; only the template bank differs:
// ordinary bank +128/+188/+1E8, options bank +248/+2A8/+308 in cursorResourceTemplates.
typedef CursorDrawBatch CursorBatchFEB0;
typedef CursorDrawRect CursorRectFEB0;
typedef CursorDrawColor CursorColorFEB0;
typedef CursorDrawState CursorStateFEB0;
struct CursorTemplatesFEB0 {
    char pad[0x128];
    CursorBatchFEB0 cursor, label, number;
};
// Build a centered screen rectangle using the stored cursor size.
inline void CursorRectInitFEB0(CursorRectFEB0& r, float x, float y) {
    r.x = x;
    r.y = y;
    r.right = kCursorSizePixels + x;
    r.bottom = kCursorSizePixels + y;
    r.x -= kCursorHalfSizePixels;
    r.right -= kCursorHalfSizePixels;
    r.y -= kCursorHalfSizePixels;
    r.bottom -= kCursorHalfSizePixels;
}
// Write positions, UVs, and RGB into a cursor template while preserving its alpha.
inline void CursorFillColorFEB0(CursorBatchFEB0& b, CursorRectFEB0& r, const CursorColorFEB0& c) {
    b.v[0].x = r.x;
    b.v[0].y = r.y;
    b.v[0].r = c.r;
    b.v[0].g = c.g;
    b.v[0].b = c.b;
    b.v[0].u = r.u;
    b.v[0].v = r.v;
    b.v[1].x = r.right;
    b.v[1].y = r.y;
    b.v[1].r = c.r;
    b.v[1].g = c.g;
    b.v[1].b = c.b;
    b.v[1].u = r.ur;
    b.v[1].v = r.v;
    b.v[2].x = r.x;
    b.v[2].y = r.bottom;
    b.v[2].r = c.r;
    b.v[2].g = c.g;
    b.v[2].b = c.b;
    b.v[2].u = r.u;
    b.v[2].v = r.vb;
    b.v[3].x = r.right;
    b.v[3].y = r.bottom;
    b.v[3].r = c.r;
    b.v[3].g = c.g;
    b.v[3].b = c.b;
    b.v[3].u = r.ur;
    b.v[3].v = r.vb;
}
// Write rectangle positions and UVs into the four cursor-template vertices.
inline void CursorFillFEB0(CursorBatchFEB0& b, CursorRectFEB0& r) {
    b.v[0].x = r.x;
    b.v[0].y = r.y;
    b.v[0].u = r.u;
    b.v[0].v = r.v;
    b.v[1].x = r.right;
    b.v[1].y = r.y;
    b.v[1].u = r.ur;
    b.v[1].v = r.v;
    b.v[2].x = r.x;
    b.v[2].y = r.bottom;
    b.v[2].u = r.u;
    b.v[2].v = r.vb;
    b.v[3].x = r.right;
    b.v[3].y = r.bottom;
    b.v[3].u = r.ur;
    b.v[3].v = r.vb;
}
// Draw a controller cursor, click animation, and the label or player number chosen by the current mode.
// player must be 0..3 for appearance/color/texture arrays. Missing device leaves
// override and appearance untouched. Valid draw consumes the shared XY override
// and resets this channel's appearance; click ticks advance on draw calls.
extern "C" void InputManager_DrawPlayerCursor(void* self, int player) {
    char* inputBytes = (char*)self;
    CursorTemplatesFEB0* templates = (CursorTemplatesFEB0*)cursorResourceTemplates;
    int layoutPlayer;

    CursorStateFEB0* state = (CursorStateFEB0*)self;
    if (!gtl_InputEx_GetScreenPointer(player))
        return;

    // Phase 1: consume override or layout position in screen pixels.
    float positionXY[2] = {kFloat_0_BalloonInput, kFloat_0_BalloonInput};

    if (player < 0)
        layoutPlayer = AsGameInputManager(inputBytes)->selectedPlayer;
    else
        layoutPlayer = player;
    if (AsGameInputManager(inputBytes)->pointerOverrideEnabled) {
        positionXY[0] = AsGameInputManager(inputBytes)->pointerOverrideX;
        positionXY[1] = AsGameInputManager(inputBytes)->pointerOverrideY;
    } else {
        void* item = (AsGameInputManager(inputBytes)->cursorLayouts)[layoutPlayer];
        if (item)
            Layout_GetCursorPosition(item, positionXY);
        else {
            positionXY[0] = positionXY[1] = kFloat_0_BalloonInput;
        }
    }
    CursorRectFEB0 rect;
    rect.u = rect.v = kFloat_0_BalloonInput;

    AsGameInputManager(inputBytes)->pointerOverrideEnabled = 0;
    rect.ur = rect.vb = kCursorUvExtent;
    CursorRectInitFEB0(rect, positionXY[0], positionXY[1]);
    Engine_SetFlexibleVertexFormat((void*)kCursorVertexFormat);

    // Phase 2: draw the cursor and advance its click-appearance timer.
    CursorColorFEB0 color = *((CursorColorFEB0*)AsGameInputManager(inputBytes)->cursorColors + player);

    CursorBatchFEB0 cursorVertices = (*(CursorBatchFEB0*)((char*)(&templates->cursor)));
    CursorFillColorFEB0(cursorVertices, rect, color);
    if (kCursorClickAppearance == state->appearance[player])
        state->tick[player] = 1;
    if (state->tick[player] && state->appearance[player] < kCursorClickAppearance)
        state->appearance[player] = kCursorClickAppearance;
    Engine_SetTextureStageResource(
        0, *(void**)((char*)((CursorResources*)inputBytes)->cursorTextures + 4 * state->appearance[player]));
    Engine_SetTexturePalette(
        0, *(void**)((char*)((CursorResources*)inputBytes)->cursorPalettes + 4 * state->appearance[player]));
    DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                    &cursorVertices, kCursorVertexBytes);
    int tick = state->tick[player];
    if (tick && tick < kCursorClickFrames)
        state->tick[player] = tick + 1;
    else
        state->tick[player] = 0;

    // Phase 3: draw the optional channel label or golf-player number below the cursor.
    // Global label mode: golf-player numbers resolve channel -> player when requested.
    switch ((AsGameInputManager(inputBytes)->cursorMode)) {
    case kCursorOnly:
        break;
    case kCursorWithLabel: {
        float width = rect.right - rect.x - kCursorLabelWidthPixels;
        rect.x = width * kCursorCenteringFactor + rect.x;
        rect.right = (kCursorLabelWidthPixels + rect.x);
        rect.y = kCursorLabelVerticalOffset + rect.bottom;
        rect.bottom = (kCursorLabelHeightPixels + rect.y);
        rect.u = rect.v = kFloat_0_BalloonInput;
        rect.ur = kCursorLabelUvWidth;
        rect.vb = kCursorLabelUvHeight;
        CursorBatchFEB0 labelVertices = (*(CursorBatchFEB0*)((char*)(&templates->label)));
        CursorFillFEB0(labelVertices, rect);
        Engine_SetTextureStageResource(0, state->labelTextures[player]);
        Engine_SetTexturePalette(0, state->labelPalettes[player]);
        DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                        &labelVertices, kCursorVertexBytes);
        break;
    }
    case kCursorWithPlayerNumber: {
        void* sharedDoc = g_sharedDoc;
        if ((void*)((CSharedDoc*)sharedDoc)->golfDoc) {
            float width = rect.right - rect.x - kCursorNumberWidthPixels;
            float delta = kFloat_0_BalloonInput;

            rect.x = width * kCursorCenteringFactor + rect.x;
            rect.right = kCursorNumberWidthPixels + rect.x;

            if (state->appearance[player] == kCursorAppearance05)
                delta -= kCursorNumberSpecialOffset;
            else
                delta -= kCursorNumberDefaultOffset;
            rect.y = (rect.bottom + delta);
            float numberBottom = kCursorNumberHeightPixels + rect.y;
            rect.u = rect.v = kFloat_0_BalloonInput;
            rect.bottom = numberBottom;
            rect.ur = rect.vb = kCursorNumberUvExtent;
            CursorBatchFEB0 numberVertices = (*(CursorBatchFEB0*)((char*)(&templates->number)));
            CursorFillFEB0(numberVertices, rect);
            if (AsGameInputManager(inputBytes)->cursorModeRequestCount) {
                unsigned int selected = 0;
                int golfPlayer = 0;
                while (golfPlayer < AsCGolfDoc((void*)((CSharedDoc*)g_sharedDoc)->golfDoc)->playerCount) {
                    if (*(int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc,
                                                          (unsigned char)golfPlayer) == player) {
                        selected = golfPlayer;
                        break;
                    }
                    ++golfPlayer;
                }
                Engine_SetTextureStageResource(
                    0, *(void**)((char*)((CursorResources*)inputBytes)->numberTextures + (4 * selected)));
                Engine_SetTexturePalette(
                    0, *(void**)((char*)((CursorResources*)inputBytes)->numberPalettes + (selected * 4)));
            } else {
                Engine_SetTextureStageResource(
                    0, *(void**)((char*)((CursorResources*)inputBytes)->numberTextures +
                                 AsCGolfDoc((void*)((CSharedDoc*)sharedDoc)->golfDoc)->currentPlayer * 4));
                Engine_SetTexturePalette(
                    0, *(void**)((char*)((CursorResources*)inputBytes)->numberPalettes +
                                 AsCGolfDoc((void*)((CSharedDoc*)g_sharedDoc)->golfDoc)->currentPlayer * 4));
            }
            DrawPrimitiveUP(kCursorTriangleStrip, GX_GetPrimitiveCount(kCursorQuadVertexCount, kCursorTriangleStrip),
                            &numberVertices, kCursorVertexBytes);
        }
    }
    }
    state->appearance[player] = 0;
}

// Read override or layout XY, substituting the supplied fallback for a missing layout.
static inline void GetPointerPositionOrFallback(char* inputBytes, float* positionXY, int player, float fallback) {
    int selectedPlayer = player < 0 ? AsGameInputManager(inputBytes)->selectedPlayer : player;
    if (AsGameInputManager(inputBytes)->pointerOverrideEnabled != 0) {
        positionXY[0] = AsGameInputManager(inputBytes)->pointerOverrideX;
        positionXY[1] = AsGameInputManager(inputBytes)->pointerOverrideY;
    } else {
        void* layout = (AsGameInputManager(inputBytes)->cursorLayouts)[selectedPlayer];
        if (layout != 0) {
            Layout_GetCursorPosition(layout, positionXY);
        } else {
            positionXY[1] = fallback;
            positionXY[0] = fallback;
        }
    }
}

// Read override or layout XY for drawing; a negative player uses the selected controller.
static inline int GetPointerPositionForDraw(char* inputBytes, float* positionXY, int player) {
    if (player < 0)
        player = (*(volatile int*)&AsGameInputManager(inputBytes)->selectedPlayer);
    if (AsGameInputManager(inputBytes)->pointerOverrideEnabled != 0) {
        positionXY[0] = AsGameInputManager(inputBytes)->pointerOverrideX;
        positionXY[1] = AsGameInputManager(inputBytes)->pointerOverrideY;
        return 1;
    }
    void* layout = (AsGameInputManager(inputBytes)->cursorLayouts)[player];
    if (layout != 0)
        return Layout_GetCursorPosition(layout, positionXY);
    float zero = kFloat_0_BalloonInput;
    positionXY[1] = zero;
    positionXY[0] = zero;
    return 0;
}

// Copy the selected RGBA color to both trail endpoints and update its color state.
static inline void ApplyCursorTrailColor(void* trail, char* colorCursor) {
    unsigned char* colors = ((CursorTrailStorage*)trail)->colors;
    colors[0] = AsGameInputManager(colorCursor)->cursorColors[0][0];
    colors[1] = AsGameInputManager(colorCursor)->cursorColors[0][1];
    colors[2] = AsGameInputManager(colorCursor)->cursorColors[0][2];
    colors[3] = AsGameInputManager(colorCursor)->cursorColors[0][3];
    colors[4] = AsGameInputManager(colorCursor)->cursorColors[0][0];
    colors[5] = AsGameInputManager(colorCursor)->cursorColors[0][1];
    colors[6] = AsGameInputManager(colorCursor)->cursorColors[0][2];
    colors[7] = AsGameInputManager(colorCursor)->cursorColors[0][3];
    TrailRibbon_SetStyle(trail, colors);
}

// Update trails and colors, honor pointer enable/suspension, and draw either the selected or all controller cursors.
// self and global mouse cursor required; selected channel 0..3, layout/trail
// vectors cover their channels. The selected trail advances even if drawing is
// below; no widescreen scale is applied locally. Preserve ribbon-before-quad
// order and the one-use override when changing coordinate mapping.
void InputManager_DrawCursors(void* self) {
    char* inputBytes = (char*)self;
    float pointerPositionPixels[2];
    // Update the selected trail even before the draw-enable/suspension gates.
    if (AsGameInputManager(inputBytes)->cursorModeRequestCount == 0) {
        int player = AsGameInputManager(inputBytes)->selectedPlayer;
        void** trails = AsGameInputManager(inputBytes)->cursorTrails;
        if (trails[player] != 0) {
            GetPointerPositionForDraw(inputBytes, pointerPositionPixels, player);
            TrailRibbon_SetPointerPosition(trails[player], pointerPositionPixels);
            TrailRibbon_Update(trails[player]);
        }
    }
    // Pointer input or an explicit override admits drawing; colors update before suspension.
    if (((CMouseCursor*)g_mouseCursor)->pointerInputEnabled == 0 &&
        AsGameInputManager(inputBytes)->pointerOverrideEnabled == 0)
        return;
    if (AsGameInputManager(inputBytes)->cursorModeRequestCount == 0) {
        InputManager_UpdateCursorColor(self, AsGameInputManager(inputBytes)->selectedPlayer);
    } else {
        for (int i = 0; i < kPlayerCount; i++)
            InputManager_UpdateCursorColor(self, i);
    }
    if (AsGameInputManager(inputBytes)->cursorDrawingSuspendCount != 0)
        return;
    // Build colored ribbons before the foreground cursor quads.
    if (AsGameInputManager(inputBytes)->cursorModeRequestCount == 0) {
        int off = AsGameInputManager(inputBytes)->selectedPlayer * 4;
        char* trails = (char*)AsGameInputManager(inputBytes)->cursorTrails;
        void* trail = *(void**)(trails + off);
        if (trail != 0) {
            ApplyCursorTrailColor(trail, inputBytes + off);
            TrailRibbon_Build(*(void**)(trails + off));
        }
    } else {
        void** trailIterator = AsGameInputManager(inputBytes)->cursorTrails;
        char* colorCursor = inputBytes;
        float zero = kFloat_0_BalloonInput;
        int i = 0;
        for (; trailIterator !=
               AsGameInputManager(inputBytes)->cursorTrails + AsGameInputManager(inputBytes)->cursorTrailCount;
             colorCursor += 4, i++, trailIterator++) {
            if (*trailIterator == 0)
                continue;
            GetPointerPositionOrFallback(inputBytes, pointerPositionPixels, i, zero);
            TrailRibbon_SetPointerPosition(*trailIterator, pointerPositionPixels);
            TrailRibbon_Update(*trailIterator);
            ApplyCursorTrailColor(*trailIterator, colorCursor);
            TrailRibbon_Build(*trailIterator);
        }
    }
    // Draw one selected channel or all channels with their individual option overrides.
    if (AsGameInputManager(inputBytes)->cursorModeRequestCount == 0) {
        int player = AsGameInputManager(inputBytes)->selectedPlayer;
        CursorDrawOptions* options = &((CursorResources*)inputBytes)->drawOptions[player];
        if (options->enabled != 0) {
            CursorDrawOptions drawOptions = *options;
            InputManager_DrawPlayerCursorWithOptions(self, &drawOptions, player);
        } else {
            InputManager_DrawPlayerCursor(self, player);
        }
    } else {
        char* optionsCursor = inputBytes;
        for (int i = 0; i < kPlayerCount; i++, optionsCursor += 0xc) {
            CursorDrawOptions* options = &((CursorResources*)optionsCursor)->drawOptions[0];
            if (options->enabled != 0) {
                CursorDrawOptions drawOptions = *options;
                InputManager_DrawPlayerCursorWithOptions(self, &drawOptions, i);
            } else {
                InputManager_DrawPlayerCursor(self, i);
            }
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
// Copy palette RGBA in channel order; explicit RGB overrides preserve alpha separately.
static inline void CopyCursorRgba(unsigned char* destination, const unsigned char* source) {
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = source[3];
}

// Choose explicit RGB override, menu palette, or golf-player palette according to cursor mode and appearance.
void InputManager_UpdateCursorColor(void* self, int player) {
    char* inputBytes = (char*)self;
    char* overrideCursor = inputBytes + player * 8;
    // Phase 1: an explicit RGB override wins and deliberately preserves alpha.
    if (((CursorResources*)overrideCursor)->colorOverrides[0].enabled != 0) {
        char* colorCursor = inputBytes + player * 4;
        AsGameInputManager(colorCursor)->cursorColors[0][0] = ((CursorResources*)overrideCursor)->colorOverrides[0].red;
        AsGameInputManager(colorCursor)->cursorColors[0][1] =
            ((CursorResources*)overrideCursor)->colorOverrides[0].green;
        AsGameInputManager(colorCursor)->cursorColors[0][2] =
            ((CursorResources*)overrideCursor)->colorOverrides[0].blue;
        return;
    }
    // Phase 2: label policy chooses shared, channel, or golf-player palette order.
    unsigned int optionsOffset = player * 0xc;
    int mode = AsGameInputManager(inputBytes)->cursorMode;
    int optionsEnabled = ((CursorResources*)inputBytes)->drawOptions[player].enabled;
    switch (mode) {
    case kCursorOnly: {
        char* colorCursor = inputBytes + player * 4;
        if (AsGameInputManager(colorCursor)->cursorAppearance[0] == kCursorActiveAppearance) {
            CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                           AsGameInputManager(inputBytes)->cursorActiveColors[0]);
        } else {
            CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                           AsGameInputManager(inputBytes)->cursorBaseColors[0]);
        }
        return;
    }
    case kCursorWithLabel: {
        char* colorCursor = inputBytes + player * 4;
        CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                       AsGameInputManager(colorCursor)->cursorAlternateColors[0]);
        return;
    }
    case kCursorWithPlayerNumber: {

        // Numbered cursors use golf-player color order, unless explicit draw options override it.
        void* golfDoc = (void*)((CSharedDoc*)g_sharedDoc)->golfDoc;
        if (golfDoc == 0)
            return;

        if (AsGameInputManager(inputBytes)->cursorModeRequestCount != 0) {
            int playerColor = 0;
            for (int golfPlayer = 0; golfPlayer < AsCGolfDoc((void*)((CSharedDoc*)g_sharedDoc)->golfDoc)->playerCount;
                 golfPlayer++) {
                if (*(int*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc,
                                                      (unsigned char)golfPlayer) == player) {
                    playerColor = golfPlayer;
                    break;
                }
            }
            char* colorCursor = inputBytes + player * 4;
            if (AsGameInputManager(colorCursor)->cursorAppearance[0] == kCursorActiveAppearance) {
                if (optionsEnabled != 0) {
                    char* base =
                        inputBytes + ((CursorResources*)(optionsOffset + inputBytes))->drawOptions[0].playerColor * 4;
                    CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                                   AsGameInputManager(base)->cursorActiveColors[0]);
                } else {
                    char* base = inputBytes + playerColor * 4;
                    CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                                   AsGameInputManager(base)->cursorActiveColors[0]);
                }
            } else {
                if (optionsEnabled != 0) {
                    char* base =
                        inputBytes + ((CursorResources*)(optionsOffset + inputBytes))->drawOptions[0].playerColor * 4;
                    CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                                   AsGameInputManager(base)->cursorBaseColors[0]);
                } else {
                    char* base = inputBytes + playerColor * 4;
                    CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                                   AsGameInputManager(base)->cursorBaseColors[0]);
                }
            }
            return;
        }

        char* colorCursor = inputBytes + player * 4;
        if (AsGameInputManager(colorCursor)->cursorAppearance[0] == kCursorActiveAppearance) {
            if (optionsEnabled != 0) {
                char* base =
                    inputBytes + ((CursorResources*)(optionsOffset + inputBytes))->drawOptions[0].playerColor * 4;
                CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                               AsGameInputManager(base)->cursorActiveColors[0]);
            } else {
                char* base = inputBytes + AsCGolfDoc(golfDoc)->currentPlayer * 4;
                CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                               AsGameInputManager(base)->cursorActiveColors[0]);
            }
        } else {
            if (optionsEnabled != 0) {
                char* base =
                    inputBytes + ((CursorResources*)(optionsOffset + inputBytes))->drawOptions[0].playerColor * 4;
                CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                               AsGameInputManager(base)->cursorBaseColors[0]);
            } else {
                char* base = inputBytes + AsCGolfDoc(golfDoc)->currentPlayer * 4;
                CopyCursorRgba(AsGameInputManager(colorCursor)->cursorColors[0],
                               AsGameInputManager(base)->cursorBaseColors[0]);
            }
        }
        return;
    }
    default:
        return;
    }
}
#pragma pop

// Query a selected-player menu action, preserving synthetic presses and both block arrays.
static inline int IsSelectedMenuActionPressed(void* self, unsigned int action) {
    GameInputManager* input = AsGameInputManager(self);
    if (((int (*)(void))GameHook_ShouldBlockInput)() != 0)
        return 0;
    if (input->actionPress[action] != 0)
        return 1;
    if (input->actionBlocked[action] != 0)
        return 0;
    if (input->actionBlockedSecondary[action] != 0)
        return 0;
    int player = input->selectedPlayer;
    void* pad = gtl_InputEx_GetScreenPointer(player);
    if (pad == 0)
        return 0;
    int category = ((int (*)(void*, unsigned int, void*))Input_MapActionToCategory)(self, action, (void*)(long)player);
    return GetButtonPressed__Q23gtl18CInputExDeviceBaseFv(pad, category);
}

// Test the six menu-advance action IDs in priority order on the selected controller.
int InputManager_IsAnyMenuAdvanceActionActive(void* self) {
    int r;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction04);
    if (r)
        return 1;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction12);
    if (r)
        return 1;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction06);
    if (r)
        return 1;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction27);
    if (r)
        return 1;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction07);
    if (r)
        return 1;
    r = IsSelectedMenuActionPressed(self, kMenuAdvanceAction08);
    return r != 0;
}

extern "C" unsigned char cursorResourceTemplates[];

// s_cursorResourcesLoadAttempted
extern "C" int s_cursorResourcesLoadAttempted;
extern "C" void LayoutCursor_InitBounds(void* p, void* q);
// Construct a controller trail with its point capacity and strip mode.
extern "C" void TrailRibbon_Ctor(void* p, void* q, int a, int b);
extern "C" void ImageResource_LoadFileAndDecode(void* a, void* b, void* c, void* d, void* e);

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Small word-array structs used only to reproduce the original by-value copy sequences:
// CursorBoundsWords = {player id, min x, min y, max x, max y (float bit patterns)};
// PlayerIdWords = per-channel player ids; PathTable4/6 = arrays of resource path pointers;
// PaletteInfoWords / ImageInfoWords = scratch descriptors passed to the image loader.
struct CursorBoundsWords {
    int player, minXBits, minYBits, maxXBits, maxYBits;
};
struct PlayerIdWords {
    int w[4];
};
struct PathTable4 {
    const void* p[4];
};
struct PathTable6 {
    const void* p[6];
};
struct PaletteInfoWords {
    int w[5];
};
struct ImageInfoWords {
    int w[4];
    int pad[5];
};

// Static pool fields used by the resource loader; drawing templates start at 0x128.
struct CursorResourcePool {
    char prefix[0x30];
    CursorBoundsWords cursorBounds;
    char reserved44[4];
    PlayerIdWords playerIds;
    char reserved58[0x68];
    PathTable6 cursorPaths;
    PathTable4 numberPaths;
    char reservedE8[0x30];
    PathTable4 labelPaths;
};
typedef char CursorResourcePool_size[(sizeof(CursorResourcePool) == 0x128) ? 1 : -1];

// Construct one layout from the copied channel and floating-point bound words.
struct CursorLayoutNew {
    char data[0x20];
    CursorLayoutNew(CursorBoundsWords* q) {
        LayoutCursor_InitBounds(this, q);
    }
};
// Construct a trail with the original point capacity and strip-mode arguments.
struct CursorTrailNew {
    char data[0x58];
    CursorTrailNew(int v, int a, int b) {
        TrailRibbon_Ctor(this, (void*)(long)v, a, b);
    }
};

// Replace cursor layouts/trails and load cursor, player-number, and label textures for the selected controller.
// self requires initialized pointer vectors and at most four channel entries.
// Invalid selectedPlayer returns 0 after setting the global attempted flag;
// existing resources stay intact. Valid input replaces layouts/trails in order,
// then loads images; there is no transactional rollback or per-image status.
// Paths/templates are borrowed static data. engine_render_cache_and_texture_load.cpp's decoder publishes OWNED
// graphics handles, each starting with an owned reference; release operation is
// GraphicsResource_Release (gx_renderer_state_and_resources.cpp), which decrements and destroys at zero.
// Draw binds acquire additional stage references and release the previous binding.
// This loader overwrites all 6+4+4 texture/palette slots WITHOUT releasing old
// owned references, and InputManager_Dtor also omits their release. This is an
// established lifetime defect, not evidence of a cache. Startup caller
// CGameMain initialization (engine_startup_and_ani_sprite.cpp) loads channel 0; repeated reloads leak.
// Image open/decode failure may clear slots, read failure leaves them unchanged,
// and palette failure may publish only a texture. All statuses are ignored here;
// return 1 means the loops ran, not that every cursor image is drawable. A future
// hot-reload mod must explicitly retain old pairs until replacement succeeds and
// release retired nonnull handles; doing so would be a separate behavior change.
// Preserve 0x18-byte XYZ/RGBA/UV vertices and atlas crops when
// replacing textures; these calls do not request a GX texture format explicitly.
int InputManager_LoadCursorResources(void* self, int selectedPlayer) {
    CursorResourcePool* resourcePool = (CursorResourcePool*)cursorResourceTemplates;
    char* inputBytes = (char*)self;
    s_cursorResourcesLoadAttempted = 1;
    if (selectedPlayer >= kPlayerCount || selectedPlayer < 0)
        return 0;
    ImageInfoWords imageInfo;
    CursorBoundsWords cursorBounds;
    PathTable6 cursorPaths;
    PaletteInfoWords paletteInfo;
    PlayerIdWords playerIds;
    PathTable4 numberPaths;
    PathTable4 labelPaths;

    {
        volatile GameInputManager* input = (volatile GameInputManager*)inputBytes;
        input->cursorAppearance[0] = 0;
        input->cursorColors[0][0] = input->cursorBaseColors[0][0];
        input->cursorColors[0][1] = input->cursorBaseColors[0][1];
        input->cursorColors[0][2] = input->cursorBaseColors[0][2];
        input->cursorColors[0][3] = input->cursorBaseColors[0][3];
        input->cursorAppearance[1] = 0;
        input->cursorColors[1][0] = input->cursorBaseColors[1][0];
        input->cursorColors[1][1] = input->cursorBaseColors[1][1];
        input->cursorColors[1][2] = input->cursorBaseColors[1][2];
        input->cursorColors[1][3] = input->cursorBaseColors[1][3];
        input->cursorAppearance[2] = 0;
        input->cursorColors[2][0] = input->cursorBaseColors[2][0];
        input->cursorColors[2][1] = input->cursorBaseColors[2][1];
        input->cursorColors[2][2] = input->cursorBaseColors[2][2];
        input->cursorColors[2][3] = input->cursorBaseColors[2][3];
        input->cursorAppearance[3] = 0;
        input->cursorColors[3][0] = input->cursorBaseColors[3][0];
        input->cursorColors[3][1] = input->cursorBaseColors[3][1];
        input->cursorColors[3][2] = input->cursorBaseColors[3][2];
        input->cursorColors[3][3] = input->cursorBaseColors[3][3];
        input->cursorMode = 0;
        input->selectedPlayer = selectedPlayer;
    }

    cursorBounds = resourcePool->cursorBounds;
    playerIds = resourcePool->playerIds;

    // Replace each layout with channel-specific screen bounds; delete old layout storage first.
    {
        void** destination = AsGameInputManager(inputBytes)->cursorLayouts;
        int* playerId = playerIds.w;
        while (destination !=
               AsGameInputManager(inputBytes)->cursorLayouts + AsGameInputManager(inputBytes)->cursorLayoutCount) {
            cursorBounds.player = *playerId;
            if (*destination != 0)
                __dl__FPv(*destination);
            *destination = new CursorLayoutNew(&cursorBounds);
            destination++;
            playerId++;
        }
    }

    // Replace trails: free each old trail's sample, vertex-array and angle buffers, then the trail itself
    // (the doubled `oldTrail != 0` test is a compiler artifact), and construct a new ribbon.
    {
        void** destination = AsGameInputManager(inputBytes)->cursorTrails;
        int* playerId = playerIds.w;
        while (destination !=
               AsGameInputManager(inputBytes)->cursorTrails + AsGameInputManager(inputBytes)->cursorTrailCount) {
            void* oldTrail = *destination;
            if (oldTrail != 0) {
                if (oldTrail != 0) {
                    void** arrays = (void**)((CursorTrailStorage*)oldTrail)->samples;
                    if (arrays != 0) {
                        __dla__FPv(arrays);
                        ((CursorTrailStorage*)oldTrail)->samples = 0;
                    }
                    for (int arrayIndex = 0; arrayIndex < ((CursorTrailStorage*)oldTrail)->arrayCount; arrayIndex++) {
                        void** arrays = ((CursorTrailStorage*)oldTrail)->sourceVertexArrays;
                        void* allocation = arrays[arrayIndex];
                        if (allocation != 0) {
                            __dla__FPv(allocation);
                            arrays = ((CursorTrailStorage*)oldTrail)->sourceVertexArrays;
                            arrays[arrayIndex] = 0;
                        }
                        arrays = ((CursorTrailStorage*)oldTrail)->animatedVertexArrays;
                        allocation = arrays[arrayIndex];
                        if (allocation != 0) {
                            __dla__FPv(allocation);
                            arrays = ((CursorTrailStorage*)oldTrail)->animatedVertexArrays;
                            arrays[arrayIndex] = 0;
                        }
                    }
                    arrays = ((CursorTrailStorage*)oldTrail)->sourceVertexArrays;
                    if (arrays != 0) {
                        __dla__FPv(arrays);
                        ((CursorTrailStorage*)oldTrail)->sourceVertexArrays = 0;
                    }
                    arrays = ((CursorTrailStorage*)oldTrail)->animatedVertexArrays;
                    if (arrays != 0) {
                        __dla__FPv(arrays);
                        ((CursorTrailStorage*)oldTrail)->animatedVertexArrays = 0;
                    }
                    void* pointAngles = ((CursorTrailStorage*)oldTrail)->pointAngles;
                    if (pointAngles != 0) {
                        __dla__FPv(pointAngles);
                        ((CursorTrailStorage*)oldTrail)->pointAngles = 0;
                    }
                    __dl__FPv(oldTrail);
                }
            }
            *destination = new CursorTrailNew(*playerId, kCursorTrailPointCapacity, kCursorTrailMode);
            destination++;
            playerId++;
        }
    }

    // Load six appearance images, then four player-number and four label image pairs.
    cursorPaths = *(PathTable6*)((char*)(&resourcePool->cursorPaths));
    numberPaths = *(PathTable4*)((char*)(&resourcePool->numberPaths));
    labelPaths = *(PathTable4*)((char*)(&resourcePool->labelPaths));
    imageInfo.w[3] = 0;
    paletteInfo.w[4] = 0;

    {
        const void** path = cursorPaths.p;
        void** paletteSlot = (void**)((char*)((CursorResources*)inputBytes)->cursorPalettes);
        void** textureSlot = (void**)((char*)((CursorResources*)inputBytes)->cursorTextures);
        for (int i = 0; i < kCursorAppearanceCount; i++) {
            void* resourcePath = CHashPath::FindPath((const char*)path[0]);
            ImageResource_LoadFileAndDecode(resourcePath, textureSlot, paletteSlot, &imageInfo, &paletteInfo);
            textureSlot++;
            path++;
            paletteSlot++;
        }
    }
    {
        const void** path = numberPaths.p;
        void** paletteSlot = (void**)((char*)((CursorResources*)inputBytes)->numberPalettes);
        void** textureSlot = (void**)((char*)((CursorResources*)inputBytes)->numberTextures);
        for (int i = 0; i < kPlayerCount; i++) {
            void* resourcePath = CHashPath::FindPath((const char*)path[0]);
            ImageResource_LoadFileAndDecode(resourcePath, textureSlot, paletteSlot, &imageInfo, &paletteInfo);
            textureSlot++;
            path++;
            paletteSlot++;
        }
    }
    {
        const void** path = labelPaths.p;
        void** paletteSlot = (void**)((char*)((CursorResources*)inputBytes)->labelPalettes);
        void** textureSlot = (void**)((char*)((CursorResources*)inputBytes)->labelTextures);
        for (int i = 0; i < kPlayerCount; i++) {
            void* resourcePath = CHashPath::FindPath((const char*)path[0]);
            ImageResource_LoadFileAndDecode(resourcePath, textureSlot, paletteSlot, &imageInfo, &paletteInfo);
            textureSlot++;
            path++;
            paletteSlot++;
        }
    }

    return 1;
}
#pragma pop

} // extern "C"
