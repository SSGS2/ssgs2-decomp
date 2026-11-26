// Menu pointer cursor for layouts: reads the Wii Remote pointer, maps it to layout coordinates,
// and gives the cursor position, bounds and depth to menus. Also sets up and tears down the
// layout controller and its pangya icon object.

#include <decomp/static_init.h>
#include "game/gtl_input.h"
// Layout controller ownership and native Wii Remote pointer-to-menu coordinates.
// Modding lookup: Layout_CursorFromPointer maps device [-1,+1] X/Y to
// [0,832]/[0,624] layout units using the 416/312 scalars in the data section.
// These are input coordinates, not EFB resolution or an aspect-ratio setting.
// LayoutCursor_InitBounds separately seeds (208,156); change mapping, bounds,
// cursor drawing and hit-test rectangles together in a later widescreen mod.
// Layout_UpdateCursorFromPlayer obtains a borrowed gtl screen-pointer device;
// no tutorial strings, swing/button mappings or controller polling rates live here.
// Layout_Init/Destroy own the resources described by LayoutResourceOwnerView.h;
// LayoutPointerCursor.h is a separate borrowed player/bounds/position record.
// Calls run in the layout lifecycle; this unit adds no synchronization.
#include "classes/LayoutResourceOwnerView.h"
#include "classes/LayoutPointerCursor.h"
#include "classes/GameInputManager.h"

#include "game/singletons.h"
#include "classes/CSPDPangyaIcon.h"
#include "classes/CSharedFileHandle.h"

// View caller-supplied icon storage without changing ownership.
inline CSPDPangyaIcon* AsCSPDPangyaIcon(void* storage) {
    return (CSPDPangyaIcon*)storage;
}

// Three independent layouts occur here: cursor bounds/position, controller resources,
// and icon children. Their void* interfaces do not make them interchangeable.
// Positive destructor flags free wrapper storage; zero/negative flags retain it.
// Repeated null checks preserve the matched destructor expansion.
// +0x20 is compared as an ordered signed word in two pointer consumers. Its
// producer's units are unresolved: do not call it a priority or a frame clock.
enum {
    kMinimumPointerSelectionRank = 1
};
extern "C" {

extern float kFloat_6_2831855;
extern float s_layoutCursorFullTurnAngle;     // Module angle storage initialized to one full turn.
extern float s_layoutControllerFullTurnAngle; // Module angle storage initialized to one full turn.
extern float kFloat_0_LayoutCursorDepth;      // 0: kFloat_0_LayoutCursorDepth.
extern float
    kFloat_416_LayoutPointerXScale; // 416 layout units: X half-span AND offset; larger expands/shifts X before clamping.
extern float
    kFloat_312_LayoutPointerYScale; // 312 layout units: Y half-span AND offset; larger expands/shifts Y before clamping.
extern float kFloat_208_LayoutCursorInitialX; // 208 layout units: reset X, independent of pointer mapping.
extern float kFloat_156_LayoutCursorInitialY; // 156 layout units: reset Y; leading float in an eight-byte data slot.
extern unsigned char __vt__Q214CSPDPangyaIcon22PANGYA_ICON_OBJECT_VAR[];
extern signed char s_inputManagerInitialized;
extern unsigned char s_gameInputManager[];
extern unsigned char s_layoutInputManagerDtorRegistration[]; // Input-manager destructor registration record.

extern void* gtl_InputEx_GetScreenPointer(void*);
extern int Layout_CursorFromPointer(void*, void*);
extern void Memory_Clear(void*, int);
extern void __destroy_new_array(void*, void*);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern void __register_global_object(void*, void (*)(), void*);
extern void __dl__FPv(void*);

struct PointerVtableObject {
    virtual void destroy(int);
};

// Initialize this cursor module angle constant from the shared full-turn value.
void LayoutCursor_InitAngleConstant() {
    s_layoutCursorFullTurnAngle = kFloat_6_2831855;
}

// Initialize this layout module angle constant from the shared full-turn value.
void LayoutController_InitAngleConstant() {
    s_layoutControllerFullTurnAngle = kFloat_6_2831855;
}

// Update a layout cursor using its player screen-pointer device.
// cursor is required; its player word is a device selector passed through the
// pointer-shaped ABI, not an owned player allocation. Returns 0 for an absent
// device and 1 even when no sample wins.
int Layout_UpdateCursorFromPlayer(void** cursor) {
    void* pointer = gtl_InputEx_GetScreenPointer(AsLayoutCursor800779EC(cursor)->player);
    if (pointer == 0) {
        return 0;
    }
    return Layout_CursorFromPointer(cursor, pointer) != 0;
}

// Initialize a layout controller with no loaded resources or registered cursor mode.
// Returns required caller-owned controller storage (at least 0x5C bytes).
// actionIdWord is copied verbatim to +48. layout.cpp's Layout_CollectActivated
// and Layout_PlayActivationSound read that word as Input_IsActionPressed's
// logical action ID. It is an integer in a pointer-shaped ABI, not a pointee.
void* Layout_Init(void* controller, void* actionIdWord) {
    AsLayoutController800779EC(controller)->activationActionId = actionIdWord;
    AsLayoutController800779EC(controller)->inputModeRegistered = 0;
    AsLayoutController800779EC(controller)->activationSoundEnabled = 1;
    (*(unsigned int*)&AsLayoutController800779EC(controller)->sharedFile) = 0;
    AsLayoutController800779EC(controller)->extendedItemTable = 0;
    (*(unsigned int*)&AsLayoutController800779EC(controller)->secondarySharedFile) = 0;
    (*(unsigned int*)&AsLayoutController800779EC(controller)->ownedObject) = 0;
    AsLayoutController800779EC(controller)->standardItems = 0;
    AsLayoutController800779EC(controller)->extendedFormat = 0;
    AsLayoutController800779EC(controller)->extendedItems = 0;
    (*(unsigned int*)&AsLayoutController800779EC(controller)->pangyaIcons) = 0;
    // +10 is the first float X word in layout.cpp's four cached pointer-position
    // pairs (+10+8*player X, +14+8*player Y). Only this first WORD is cleared
    // here; Layout_UpdateStandard/Extended fill positions through InputManager
    // before hit testing. Keep the four-byte clear, rather than clearing the array.
    Memory_Clear(&AsLayoutController800779EC(controller)->cachedPointerX0Bits, sizeof(unsigned int));
    AsLayoutController800779EC(controller)->loadState = 0;
    return controller;
}

// Delete the four owned icon children, then optionally free the icon storage.
// Null is allowed; return is the original address, possibly freed when deleting > 0.
void* CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor(void* icon, int deleting) {
    if (icon != 0) {
        *(void**)icon = __vt__Q214CSPDPangyaIcon22PANGYA_ICON_OBJECT_VAR;
        void* child = AsCSPDPangyaIcon(icon)->ownedChild0;
        if (child != 0) {
            if (child != 0) {
                ((PointerVtableObject*)child)->destroy(1);
            }
            AsCSPDPangyaIcon(icon)->ownedChild0 = 0;
        }
        child = AsCSPDPangyaIcon(icon)->ownedChild1;
        if (child != 0) {
            if (child != 0) {
                ((PointerVtableObject*)child)->destroy(1);
            }
            AsCSPDPangyaIcon(icon)->ownedChild1 = 0;
        }
        child = AsCSPDPangyaIcon(icon)->ownedChild2;
        if (child != 0) {
            if (child != 0) {
                ((PointerVtableObject*)child)->destroy(1);
            }
            AsCSPDPangyaIcon(icon)->ownedChild2 = 0;
        }
        child = AsCSPDPangyaIcon(icon)->ownedChild3;
        if (child != 0) {
            if (child != 0) {
                ((PointerVtableObject*)child)->destroy(1);
            }
            AsCSPDPangyaIcon(icon)->ownedChild3 = 0;
        }
        if (deleting > 0) {
            __dl__FPv(icon);
        }
    }
    return icon;
}

// Copy the current layout-space cursor coordinates to the output pair.
// positionOut needs two floats [x,y]; null output returns 0, otherwise returns 1. cursor is borrowed.
int Layout_GetCursorPosition(void* cursor, float* positionOut) {
    if (positionOut == 0) {
        return 0;
    }
    float cursorY;
    float cursorX;
    cursorX = AsLayoutCursor800779EC(cursor)->cursorX;
    cursorY = AsLayoutCursor800779EC(cursor)->cursorY;
    positionOut[0] = cursorX;
    positionOut[1] = cursorY;
    return 1;
}

// Select the greatest sample ordering word, map X/Y, and clamp to the layout bounds.
// Requires a valid cursor; returns 0 only for a null pointer device. With no rank >= 1
// sample, returns 1 and leaves all cursor values unchanged. Depth is copied in device units.
// Normalized x/y map as (x+1)*416, (y+1)*312, then clamp to the caller's layout rectangle.
// Positive device axes map to increasing layout axes; this path does not invert Y.
// Require minX<=maxX and minY<=maxY for a conventional clamp. Safe tuning ranges
// are unestablished. GetDeviceResult's status is ignored in the existing ABI;
// the device must still supply a count. Preserve that call and backward traversal.
int Layout_CursorFromPointer(void* cursor, void* pointer) {
    if (pointer == 0)
        return 0;

    int sampleIndex;
    ((gtl::CInputExDeviceBase*)pointer)->GetDeviceResult(&sampleIndex);
    struct Position {
        float x;
        float y;
        float z;
    } position = {};
    int selectionCount = 0;
    int bestSelectionRank = kMinimumPointerSelectionRank;
    float screenHalfHeight;
    float screenHalfWidth;
    float depth;
    depth = kFloat_0_LayoutCursorDepth;
    screenHalfWidth = kFloat_416_LayoutPointerXScale;
    screenHalfHeight = kFloat_312_LayoutPointerYScale;

    // Scan backwards: accepting equal ranks makes the lowest index win a tie.
    // Hit payload: +0x0c/+0x10 normalized x/y, +0x1c depth, +0x20 ordering word;
    // the other five words have no established interpretation in this consumer.
    while (--sampleIndex >= 0) {
        struct Hit {
            int a[3];
            float x;
            float y;
            int b[2];
            float depth;
            int selectionRank;
        } hit = {};
        if (((gtl::CScreenPointer*)pointer)->GetSample(sampleIndex, (gtl::ScreenPointerSample*)&hit)) {
            if (hit.selectionRank >= bestSelectionRank) {

                ++selectionCount;
                bestSelectionRank = hit.selectionRank;
                position.x = hit.x;
                position.y = hit.y;
                depth = hit.depth;
                position.y = screenHalfHeight * position.y + screenHalfHeight;
                position.x = screenHalfWidth * position.x + screenHalfWidth;
            }
        }
    }
    // Commit only after at least one selection; no averaging of the accepted samples occurs.
    if (selectionCount) {
        LayoutPointerCursor* cursorState = AsLayoutCursor800779EC(cursor);
        AsLayoutCursor800779EC(cursorState)->cursorX = position.x;
        AsLayoutCursor800779EC(cursorState)->cursorY = position.y;
        if (AsLayoutCursor800779EC(cursorState)->minX > AsLayoutCursor800779EC(cursorState)->cursorX)
            AsLayoutCursor800779EC(cursorState)->cursorX = AsLayoutCursor800779EC(cursorState)->minX;
        if (AsLayoutCursor800779EC(cursorState)->maxX < AsLayoutCursor800779EC(cursorState)->cursorX)
            AsLayoutCursor800779EC(cursorState)->cursorX = AsLayoutCursor800779EC(cursorState)->maxX;
        if (AsLayoutCursor800779EC(cursorState)->minY > AsLayoutCursor800779EC(cursorState)->cursorY)
            AsLayoutCursor800779EC(cursorState)->cursorY = AsLayoutCursor800779EC(cursorState)->minY;
        if (AsLayoutCursor800779EC(cursorState)->maxY < AsLayoutCursor800779EC(cursorState)->cursorY)
            AsLayoutCursor800779EC(cursorState)->cursorY = AsLayoutCursor800779EC(cursorState)->maxY;
        AsLayoutCursor800779EC(cursorState)->cursorDepth = depth;
    }
    return 1;
}

// Release layout resources and its registered input cursor-mode request.
// Null is allowed. deleting > 0 frees the controller; return is its original, possibly freed address.
// The input manager is shared and retained; inputModeRegistered owns one request-count decrement.
void* Layout_Destroy(void* controller, int deleting) {
    if (controller != 0) {
        void** state = (void**)controller;
        // Release the individually owned object, both shared-file wrappers, and the icon array.
        if (AsLayoutController800779EC(state)->ownedObject != 0) {
            if (AsLayoutController800779EC(state)->ownedObject != 0) {
                ((PointerVtableObject*)AsLayoutController800779EC(state)->ownedObject)->destroy(1);
            }
            AsLayoutController800779EC(state)->ownedObject = 0;
        }
        if (AsLayoutController800779EC(state)->sharedFile != 0) {
            CSharedFileHandle::Destroy(AsLayoutController800779EC(state)->sharedFile, 1);
            AsLayoutController800779EC(state)->sharedFile = 0;
        }
        if (AsLayoutController800779EC(state)->secondarySharedFile != 0) {
            CSharedFileHandle::Destroy(AsLayoutController800779EC(state)->secondarySharedFile, 1);
            AsLayoutController800779EC(state)->secondarySharedFile = 0;
        }
        if (AsLayoutController800779EC(state)->pangyaIcons != 0) {
            __destroy_new_array(AsLayoutController800779EC(state)->pangyaIcons,
                                (void*)CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor);
            AsLayoutController800779EC(state)->pangyaIcons = 0;
        }
        // Lazily obtain the shared input manager, then relinquish this controller's cursor-mode request.
        if (AsLayoutController800779EC(state)->inputModeRegistered != 0) {
            if (((int&)GetInputManagerSlot()) == 0) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, InputManager_Dtor,
                                             s_layoutInputManagerDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
            }
            --((GameInputManager*)((int&)GetInputManagerSlot()))->cursorModeRequestCount;
        }
        if (deleting > 0)
            __dl__FPv(controller);
    }
    return controller;
}

// Initialize icon ownership pointers and primary/alternate child selectors.
// Caller supplies storage; no child allocations occur here.
void CSPDPangyaIcon_PangyaIconObject_ctor(void* icon) {
    *(void**)icon = __vt__Q214CSPDPangyaIcon22PANGYA_ICON_OBJECT_VAR;
    (*(unsigned int*)&AsCSPDPangyaIcon(icon)->ownedChild0) = 0;
    (*(unsigned int*)&AsCSPDPangyaIcon(icon)->ownedChild1) = 0;
    (*(unsigned int*)&AsCSPDPangyaIcon(icon)->ownedChild2) = 0;
    (*(unsigned int*)&AsCSPDPangyaIcon(icon)->ownedChild3) = 0;
    // layout.cpp's Layout_GetElementValue selects ownedChild0..3 using +58 for
    // alt==0 and +59 for alt!=0 in the extended format. Defaults choose child
    // 0 for primary value, child 1 for alternate/feedback; valid selectors 0..3.
    AsCSPDPangyaIcon(icon)->primaryChildIndex = 0;
    AsCSPDPangyaIcon(icon)->alternateChildIndex = 1;
}

#pragma opt_propagation off
// Initialize the cursor position and optionally copy player and rectangle bounds.
// Position always becomes (208,156) layout units; depth is untouched. Null bounds leaves player/limits unchanged.
// bounds uses the cursor-prefix layout: device selector, then minX/minY/maxX/maxY in layout units.
// Required cursor and optional bounds may alias: X/Y writes precede the prefix copy.
void LayoutCursor_InitBounds(void* cursor, void* bounds) {
    float initialY;
    float initialX;
    initialX = kFloat_208_LayoutCursorInitialX;
    initialY = kFloat_156_LayoutCursorInitialY;
    AsLayoutCursor800779EC(cursor)->cursorX = initialX;
    AsLayoutCursor800779EC(cursor)->cursorY = initialY;
    if (bounds == 0) {
        return;
    }
    (*(unsigned int*)&AsLayoutCursor800779EC(cursor)->player) =
        (*(unsigned int*)&AsLayoutCursor800779EC(bounds)->player);
    // Copy float representations as words to preserve the original non-FPU load/store sequence.
    unsigned int minYBits;
    unsigned int minXBits;
    minXBits = (*(unsigned int*)&AsLayoutCursor800779EC(bounds)->minX);
    minYBits = (*(unsigned int*)&AsLayoutCursor800779EC(bounds)->minY);
    (*(unsigned int*)&AsLayoutCursor800779EC(cursor)->minX) = minXBits;
    (*(unsigned int*)&AsLayoutCursor800779EC(cursor)->minY) = minYBits;
    unsigned int maxYBits;
    unsigned int maxXBits;
    maxXBits = (*(unsigned int*)&AsLayoutCursor800779EC(bounds)->maxX);
    maxYBits = (*(unsigned int*)&AsLayoutCursor800779EC(bounds)->maxY);
    (*(unsigned int*)&AsLayoutCursor800779EC(cursor)->maxX) = maxXBits;
    (*(unsigned int*)&AsLayoutCursor800779EC(cursor)->maxY) = maxYBits;
}
#pragma opt_propagation reset

// Copy the depth of the chosen pointer sample to the output.
// Null depthOut returns 0; otherwise copies one float and returns 1. Depth units remain device-defined.
int Layout_GetCursorDepth(void* cursor, float* depthOut) {
    if (depthOut == 0) {
        return 0;
    }
    *depthOut = AsLayoutCursor800779EC(cursor)->cursorDepth;
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462008", _ctors_u800779EC_0, LayoutCursor_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462008", _ctors_u800779EC_1, LayoutController_InitAngleConstant);
#pragma pop
