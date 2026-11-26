// The in-game tutorial dialog (CInGameTutorialDialog): tutorial pages with their shared layout,
// the cursor and movable element groups, and the result buttons.
//
// Also holds the golf cup preview (CGolfCup) that the tutorial shows with its mesh.

#include <decomp/static_init.h>
#include "classes/CGolfCup.h"
#include "classes/CInGameTutorialDialog.h"
#include "classes/CMouseCursor.h"
#include "classes/CInGameDialog.h"

// Borrow the dialog layout view; the caller retains the object.
static inline CInGameTutorialDialog* AsCInGameTutorialDialog(void* object) {
    return (CInGameTutorialDialog*)object;
}

extern "C" {
enum {
    kTutorialCount = 7,
    kTutorialElementCount = 32,
    kTutorialBodyElementCount = 18,
    kTutorialCursorGroupCount = 2,
    kTutorialMovableGroupCount = 5,
    kDialogAcceptingInput = 4,
    kTutorialLayoutKind = 0x18,
    kTutorialPageMaskWordOffset = 0x4a0 / 4, // enabledByTutorial within the table, in words
    kLayoutNeutralPlayerColor = 5
};
struct TutorialLayoutTables {
    char resourceStrings[0x4A0];
    int enabledByTutorial[kTutorialCount][kTutorialElementCount];
    int primaryCursorElements[4];
    int secondaryCursorElements[3];
    int reserved83C;
    int bodyElements[kTutorialBodyElementCount];
    int movableElements[kTutorialMovableGroupCount];
    int reserved89C;
    int textAnchorElements[4];
};
extern const TutorialLayoutTables s_tutorialLayoutTables;
typedef char TutorialLayoutTables_body[((int)&((TutorialLayoutTables*)0)->bodyElements == 0x840) ? 1 : -1];
typedef char TutorialLayoutTables_text[((int)&((TutorialLayoutTables*)0)->textAnchorElements == 0x8A0) ? 1 : -1];
// Page mask contract: enabledByTutorial[tutorialIndex][layoutElementId],
// tutorialIndex in [0,7), element IDs in [0,32). Each named ID array below
// selects a family; actual control identities come from the shared layout data.
// Both groups share the x/y, player-flag, enabled layout used by the class header.
struct TutorialElementGroup {
    float x, y;
    int playerFlag;
    int enabled;
};

void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Arm cleanup for the temporary activated-ID vector.
    explicit ActivatedUnwindCleanup(volatile void* vector) : words((volatile unsigned int*)vector), active(true) {}
    // Disarm unwind cleanup before the explicit normal cleanup.
    void release() {
        active = false;
    }
    // Free the activated-ID buffer only while this guard remains armed.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};

extern void* memset(void* dst, int value, unsigned int size);

extern void* g_tutorialSharedLayoutReady;
extern const int kTutorialPrimaryButtonResult[2];
extern const int kTutorialSecondaryButtonResult;
extern const float kTutorialMinimumDrawOpacity;
extern int s_tutorialTextAnchorElementIds[4];
extern int s_tutorialElementVisibilityByPage[][32];
extern const float kFloat_6_2831855;
extern float s_tutorialDialogFullTurn;
extern float s_golfCupFullTurn;
extern const float kTutorialCursorMoveSentinel;
extern const int kTutorialButtonScaleX;
extern const int kTutorialButtonScaleY;
extern float s_tutorialElementPositions[];
extern float zeroInitializationWords[];
extern float s_tutorialPageDimensions[];
extern const int s_tutorialSecondaryElementIds;
extern const int s_tutorialBodyElementIds[];
extern void* g_mouseCursor;
extern void* g_tutorialSharedLayout;
extern const float kGolfCupPreviewMeshScale;
extern const float kFloat_0_GolfCup;
extern void* g_golfCupPreview;
extern char s_golfCupBaseVtable[];
extern char __vt__8CGolfCup[];
extern char __vt__21CInGameTutorialDialog[];

void Layout_Destroy(void* arg0, int arg1);
void __dt__15SmallDialogBaseFv(void* arg0, int arg1);
void __dl__FPv(void* arg0);
void* __nw__FUl(unsigned int size);
void Layout_Init(void* arg0, int arg1);
void Layout_InitStandard(void* arg0, int arg1, void* arg2, int arg3, const char* arg4);
int __ptmf_test(void* arg0);
int __ptmf_scall(void* arg0, void* arg1, void* arg2);
void Layout_Update(void* self);
void* Layout_PollLoadComplete(void* self);
void Layout_SetElementScale(void* self, int index, void* point);
void Layout_GetElementPosition(void* self, int index, void* point);
void BaseObject_ctor(void* self);
void PSMTX44Identity(void* self);
void GolfCupMesh_CreateScaledTriangles(void* self, void* arg1, int arg2, int arg3, float arg4, float arg5, float arg6);
int Layout_CollectActivated(void* self, void* out, int arg2);
void appFreeUni__FPv(void* self);
void Layout_DrawWithOpacity(void* self, float time);
int InGameDialog_GetNonemptyWideText(void* self);
int Dialog_GetOpacityByte(void* self);
void DialogText_DrawAligned(void* self, void* point, int index, int arg3, int arg4);
void gtl_Vec2_Copy(void* self, void* arg1);
void Layout_SetElementPosition(void* self, void* arg1, void* arg2);
void Layout_GetElementSize(void* self, void* arg1, void* arg2);
void Layout_WriteElementRect(void* self, void* arg1, void* arg2);

void __dt__11GolfCupListFv(void* self);
void __dt__15GolfCupMatricesFv(void* self);
void __dt__11GolfCupBaseFv(void* self);
extern char s_tutorialLayoutElements[];
extern const char s_tutorialLayoutScriptFilename[];
extern void Layout_SetElementEnabled(void* self, int index, int visible);
extern void Layout_SetElementPlayerFlag(void* self, int index, int visible, int colour);
extern void Layout_SetElementPlayerFlagsAndState(void* self, int index, int visible);

// Poll shared layout loading and cache the 32 element positions when it becomes ready.
void* TutorialDialog_PollSharedLayout() {
    if (g_tutorialSharedLayout == 0) {
        return 0;
    }
    if (g_tutorialSharedLayoutReady == 0) {
        Layout_Update(g_tutorialSharedLayout);
        g_tutorialSharedLayoutReady = Layout_PollLoadComplete(g_tutorialSharedLayout);
        if (g_tutorialSharedLayoutReady != 0) {
            int point[2] = {kTutorialButtonScaleX, kTutorialButtonScaleY};
            Layout_SetElementScale(g_tutorialSharedLayout, 3, point);
            Layout_SetElementScale(g_tutorialSharedLayout, 6, point);
            Layout_SetElementScale(g_tutorialSharedLayout, 9, point);
            Layout_SetElementScale(g_tutorialSharedLayout, 12, point);
            float* item = s_tutorialElementPositions;
            int i = 0;
            for (; i < kTutorialElementCount; ++i, item += 2) {
                Layout_GetElementPosition(g_tutorialSharedLayout, i, item);
            }
        }
    }
    return g_tutorialSharedLayoutReady;
}

// Reset the dialog origin, both cursor groups and five initially hidden movable groups.
void CInGameTutorialDialog::InitGroups() {
    float* data = s_tutorialElementPositions;
    struct {
        int a;
        int b;
    } pair;
    pair.a = ((int*)data)[4];
    pair.b = ((int*)data)[5];
    float f0 = *(float*)&pair.a;
    float f1 = *(float*)&pair.b;
    positionX = f0;
    positionY = f1;
    cursorGroup0X = data[0];
    cursorGroup0Y = data[1];
    cursorGroup0PlayerFlag = 1;
    cursorGroup0Enabled = 1;
    cursorGroup1X = data[2];
    cursorGroup1Y = data[3];
    cursorGroup1PlayerFlag = 1;
    cursorGroup1Enabled = 1;
    elementGroup0X = data[30];
    elementGroup0Y = data[31];
    elementGroup0PlayerFlag = 0;
    elementGroup0Enabled = 0;
    elementGroup1X = data[32];
    elementGroup1Y = data[33];
    elementGroup1PlayerFlag = 0;
    elementGroup1Enabled = 0;
    elementGroup2X = data[34];
    elementGroup2Y = data[35];
    elementGroup2PlayerFlag = 0;
    elementGroup2Enabled = 0;
    elementGroup3X = data[36];
    elementGroup3Y = data[37];
    elementGroup3PlayerFlag = 0;
    elementGroup3Enabled = 0;
    elementGroup4X = data[38];
    elementGroup4Y = data[39];
    elementGroup4PlayerFlag = 0;
    elementGroup4Enabled = 0;
}

// Initialize a tutorial page, callback state, group storage and its per-page dimensions.
CInGameTutorialDialog::CInGameTutorialDialog(void* arg1) {
    CInGameDialog::Init((InGameDialogView*)this);
    void* lblptr = __vt__21CInGameTutorialDialog;
    int* g = (int*)zeroInitializationWords;
    tutorialIndex = (int)arg1;
    dialogVtable = lblptr;
    selectionResult = 0;
    callbackThisAdjustment = g[0];
    callbackVtableOffset = g[1];
    callbackFunction = g[2];
    callbackTarget = 0;
    unk_64 = -1; // unset index; its consumer is outside this unit
    if ((int)arg1 < 0 || (int)arg1 >= kTutorialCount)
        tutorialIndex = 0;
    char* cursor = ((char*)&cursorGroup0X);
    unsigned int i = 0;
    do {
        memset(cursor, 0, sizeof(TutorialElementGroup));
        ++i;
        cursor += sizeof(TutorialElementGroup);
    } while (i < kTutorialCursorGroupCount);
    unsigned int j = 0;
    char* cursor2 = ((char*)&elementGroup0X);
    do {
        memset(cursor2, 0, sizeof(TutorialElementGroup));
        ++j;
        cursor2 += sizeof(TutorialElementGroup);
    } while (j < kTutorialMovableGroupCount);
    ((void (*)(void*))((*(void***)&dialogVtable))[0x84 / 4])(this);
    int index = tutorialIndex;
    float* base = s_tutorialPageDimensions;
    float* entry = (float*)((char*)base + index * 8);
    float a = base[index * 2];
    float b = entry[1];
    tutorialWidth = a;
    tutorialHeight = b;
}

// Destroy the shared tutorial layout and clear its readiness flag.
void TutorialLayout_DestroySharedResource() {
    if (g_tutorialSharedLayout != 0) {
        Layout_Destroy(g_tutorialSharedLayout, 1);
        g_tutorialSharedLayout = 0;
    }
    g_tutorialSharedLayoutReady = 0;
}

// Destroy the base dialog and optionally free this allocation.
CInGameTutorialDialog::~CInGameTutorialDialog() {
    __dt__15SmallDialogBaseFv(this, 0);
}

struct DialogBase4 {
    int primaryVtable;
};
class DialogV4 : public DialogBase4 {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10(int, int*);
    virtual int v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58(void*, void*);
    virtual void v5C();
    virtual void v60();
    virtual void v64(float*, float*);
    virtual void v68(float*, float*);
    virtual void v6C();
    virtual void v70();
    virtual void v74(int, float*);
    virtual void v78(float*);
    virtual void v7C(float*);
    virtual void v80();
};

// Dispatch the content-reset hook; slot names remain ABI placeholders.
static inline void ResetTutorialContent(void* dialog) {
    ((DialogV4*)dialog)->v54();
}
// Pass the caller-owned content arguments to the content hook without retaining them here.
static inline void ApplyTutorialContent(void* dialog, void* first, void* second) {
    ((DialogV4*)dialog)->v58(first, second);
}
// Reset the position hook before applying the two layout hooks.
static inline void ResetTutorialPosition(void* dialog) {
    ((DialogV4*)dialog)->v60();
}
// Apply both position hooks in their original order; argument roles beyond XY remain unresolved.
static inline void ApplyTutorialPosition(void* dialog, float* first, float* second) {
    ((DialogV4*)dialog)->v64(first, second);
    ((DialogV4*)dialog)->v68(first, second);
}

// Reset page-specific content, then apply the caller's content arguments.
void CInGameTutorialDialog::ResetAndApplyContent(void* arg1, void* arg2) {
    ResetTutorialContent(this);
    ApplyTutorialContent(this, arg1, arg2);
}

// Reset positioning and lay out the page's body and cursor elements.
void CInGameTutorialDialog::PositionPageElements(float* arg1, float* arg2) {
    ResetTutorialPosition(this);
    ApplyTutorialPosition(this, arg1, arg2);
}

// Position the two secondary elements relative to the cached secondary origin.
void CInGameTutorialDialog::PositionSecondaryElements(float* pos) {
    float point[2] = {};
    for (unsigned int i = 0; i < 2; ++i) {
        int index = *(&s_tutorialSecondaryElementIds + i);
        float x = pos[0] + s_tutorialElementPositions[index * 2];
        float y = pos[1] + s_tutorialElementPositions[index * 2 + 1];
        point[0] = x - s_tutorialElementPositions[0x70 / 4];
        point[1] = y - s_tutorialElementPositions[0x74 / 4];
        ((DialogV4*)this)->v74(index, point);
    }
}

// Position the 18 body elements relative to the cached dialog origin.
void CInGameTutorialDialog::PositionBodyElements(float* pos) {
    float point[2] = {};
    for (unsigned int i = 0; i < kTutorialBodyElementCount; ++i) {
        int index = s_tutorialBodyElementIds[i];
        float x = pos[0] + s_tutorialElementPositions[index * 2];
        float y = pos[1] + s_tutorialElementPositions[index * 2 + 1];
        point[0] = x - s_tutorialElementPositions[0x10 / 4];
        point[1] = y - s_tutorialElementPositions[0x14 / 4];
        ((DialogV4*)this)->v74(index, point);
    }
}

// Return the shared layout readiness token.
void* CInGameTutorialDialog::GetSharedLayoutReady() {
    return g_tutorialSharedLayoutReady;
}

// Initialize this source group's full-circle constant.
void TutorialDialog_InitFullTurnConstant() {
    s_tutorialDialogFullTurn = kFloat_6_2831855;
}

struct V8Target {};
// Callback ABI: +0x50 is the integer result/context, +0x54 is a 12-byte
// CodeWarrior member pointer, +0x60 is the borrowed receiver. Signature:
// int receiver::callback(int context, void* dialog). Neither object is transferred.
// Missing callback/receiver accepts; returning zero vetoes closing the dialog.
// The game data holds a 12-byte CodeWarrior member pointer.
struct V8Obj {
    char pad0[0x50];
    int arg;
    int (V8Target::*pmf)(int, void*);
    V8Target* obj;
};

// Invoke the optional result callback; a false callback result keeps the dialog open.
int CInGameTutorialDialog::InvokeResultCallback() {
    V8Obj* callbackView = (V8Obj*)this;
    if (__ptmf_test(((char*)&callbackThisAdjustment)) != 0 && callbackView->obj != 0 &&
        (callbackView->obj->*callbackView->pmf)(callbackView->arg, this) == 0) {
        return 0;
    }
    return 1;
}

struct Obj_80077DB4 {
    char layoutStorage[0x5C];
    // Initialize the shared tutorial layout storage for the requested kind.
    Obj_80077DB4(int layoutKind) {
        Layout_Init(this, layoutKind);
    }
};

// Create and start loading the 32-element shared tutorial layout once.
void TutorialLayout_LoadSharedResource() {
    if (g_tutorialSharedLayout == 0) {
        Obj_80077DB4* object = new Obj_80077DB4(kTutorialLayoutKind);
        g_tutorialSharedLayout = object;
        Layout_InitStandard(object, 0, s_tutorialLayoutElements, kTutorialElementCount, s_tutorialLayoutScriptFilename);
    }
}

struct CInGameTutorialDialogStorage {
    char dialogStorage[0xD8];
    // Initialize a new tutorial dialog with its borrowed constructor context.
    CInGameTutorialDialogStorage(void* a) {
        ((CInGameTutorialDialog*)this)->__ct(a);
    }
};

// Allocate a tutorial page and return its pointer through a required output slot.
int CInGameTutorialDialog::Create(void** out) {
    if (out == 0) {
        return 0;
    }
    CInGameTutorialDialogStorage* object = new CInGameTutorialDialogStorage(this);
    *out = object;
    return object != 0;
}

// Borrow bodyElements from the shared tutorial layout table; element IDs index the layout.
static inline int* TutorialBodyElementIds(int* tableBase) {
    return (int*)((char*)(((TutorialLayoutTables*)tableBase)->bodyElements));
}
// Borrow movableElements from the shared tutorial layout table; element IDs index the layout.
static inline int* TutorialMovableElementIds(int* tableBase) {
    return (int*)((char*)(((TutorialLayoutTables*)tableBase)->movableElements));
}
// Borrow primaryCursorElements from the shared tutorial layout table; element IDs index the layout.
static inline int* TutorialPrimaryCursorIds(int* tableBase) {
    return (int*)((char*)(((TutorialLayoutTables*)tableBase)->primaryCursorElements));
}
// Borrow secondaryCursorElements from the shared tutorial layout table; element IDs index the layout.
static inline int* TutorialSecondaryCursorIds(int* tableBase) {
    return (int*)((char*)(((TutorialLayoutTables*)tableBase)->secondaryCursorElements));
}
// Borrow textAnchorElements from the shared tutorial layout table; element IDs index the layout.
static inline int* TutorialTextAnchorIds(int* tableBase) {
    return (int*)((char*)(((TutorialLayoutTables*)tableBase)->textAnchorElements));
}

// Read a page/element mask; both indices must satisfy the table contract above.
static inline int TutorialElementMask(int* table, int pageIndex, int elementId) {
    return ((int (*)[kTutorialElementCount])table)[pageIndex][elementId];
}

// Read a cursor mask with element-first address arithmetic, preserving operand order.
static inline int TutorialCursorElementMask(int* table, int pageIndex, int elementId) {
    return *(int*)((char*)table + elementId * sizeof(int) + pageIndex * (kTutorialElementCount * sizeof(int)));
}

#pragma opt_lifetimes off
// Apply page visibility masks, group positions and player flags to the shared layout.
// The page-mask base is rebuilt from the table in each phase; the original compiles that way.
void CInGameTutorialDialog::RefreshLayoutVisibility() {
    // Phases: apply page masks to body/secondary IDs, combine movable-group
    // flags with those masks, update both cursor families, then reset text anchors.
    int* base = (int*)&s_tutorialLayoutTables;
    if (g_tutorialSharedLayout != 0) {
        // Phase 1: movableXY the body, then apply its page mask.
        float point[2];
        float y, x;
        x = positionX;
        y = positionY;
        point[0] = x;
        point[1] = y;
        ((DialogV4*)this)->v78(point);
        int* elementIds = TutorialBodyElementIds(base);
        int* pageMasks = base + kTutorialPageMaskWordOffset;
        for (unsigned int i = 0; i < kTutorialBodyElementCount; ++i, ++elementIds) {
            int tutorialPageIndex = tutorialIndex;
            int elementId = *elementIds;
            Layout_SetElementEnabled(g_tutorialSharedLayout, elementId,
                                     TutorialElementMask(pageMasks, tutorialPageIndex, elementId));
        }
        // Phase 2: position and mask the secondary element family.
        ((DialogV4*)this)->v7C(point);
        pageMasks = base + kTutorialPageMaskWordOffset;
        const int* smallItems = &s_tutorialSecondaryElementIds;
        for (unsigned int i = 0; i < 2; ++i, ++smallItems) {
            int tutorialPageIndex = tutorialIndex;
            int elementId = *smallItems;
            Layout_SetElementEnabled(g_tutorialSharedLayout, elementId,
                                     TutorialElementMask(pageMasks, tutorialPageIndex, elementId));
        }
        // Phase 3: move and mask the five independent groups.
        // Five movable groups occupy 0x10 bytes each: x,y,playerFlag,enabled.
        // movableOwnerBase rebases only elementGroup0Enabled while movableXY walks matching x/y.
        // Visibility = page mask AND group.enabled; movable player color is neutral.
        char* movableOwnerBase = (char*)this;
        elementIds = TutorialMovableElementIds(base);
        float* movableXY = (&elementGroup0X);
        pageMasks = base + kTutorialPageMaskWordOffset;
        for (int i = 0; i < kTutorialMovableGroupCount; ++i, movableXY += 4, movableOwnerBase += 0x10, ++elementIds) {
            int elementId = *elementIds;
            ((DialogV4*)this)->v74(elementId, movableXY);
            int tutorialPageIndex = tutorialIndex;
            Layout_SetElementEnabled(g_tutorialSharedLayout, elementId,
                                     TutorialElementMask(pageMasks, tutorialPageIndex, elementId) != 0 &&
                                         ((CInGameTutorialDialog*)movableOwnerBase)->elementGroup0Enabled != 0);
            Layout_SetElementPlayerFlag(g_tutorialSharedLayout, elementId, 0, kLayoutNeutralPlayerColor);
        }
        // Phase 4: position, enable and highlight both cursor families.
        // Cursor groups share one XY position across their 4/3 element IDs. Page masks
        // gate both enabled and playerFlag; these two controls are independent.
        elementIds = TutorialPrimaryCursorIds(base);
        pageMasks = base + kTutorialPageMaskWordOffset;
        for (unsigned int i = 0; i < 4; ++i, ++elementIds) {
            int elementId = *elementIds;
            ((DialogV4*)this)->v74(elementId, (&cursorGroup0X));
            int tutorialPageIndex = tutorialIndex;
            Layout_SetElementEnabled(g_tutorialSharedLayout, elementId,
                                     TutorialCursorElementMask(pageMasks, tutorialPageIndex, elementId) != 0 &&
                                         cursorGroup0Enabled != 0);
            tutorialPageIndex = tutorialIndex;
            Layout_SetElementPlayerFlag(g_tutorialSharedLayout, elementId,
                                        TutorialCursorElementMask(pageMasks, tutorialPageIndex, elementId) != 0 &&
                                            cursorGroup0PlayerFlag != 0,
                                        kLayoutNeutralPlayerColor);
        }
        elementIds = TutorialSecondaryCursorIds(base);
        pageMasks = base + kTutorialPageMaskWordOffset;
        for (unsigned int i = 0; i < 3; ++i, ++elementIds) {
            int elementId = *elementIds;
            ((DialogV4*)this)->v74(elementId, (&cursorGroup1X));
            int tutorialPageIndex = tutorialIndex;
            Layout_SetElementEnabled(g_tutorialSharedLayout, elementId,
                                     TutorialCursorElementMask(pageMasks, tutorialPageIndex, elementId) != 0 &&
                                         cursorGroup1Enabled != 0);
            tutorialPageIndex = tutorialIndex;
            Layout_SetElementPlayerFlag(g_tutorialSharedLayout, elementId,
                                        TutorialCursorElementMask(pageMasks, tutorialPageIndex, elementId) != 0 &&
                                            cursorGroup1PlayerFlag != 0,
                                        kLayoutNeutralPlayerColor);
        }
        // Phase 5: reset the four text-anchor states.
        elementIds = TutorialTextAnchorIds(base);
        for (unsigned int i = 0; i < 4; ++i, ++elementIds) {
            int elementId = *elementIds;
            Layout_SetElementPlayerFlagsAndState(g_tutorialSharedLayout, elementId, 0);
        }
    }
}
#pragma opt_lifetimes reset

// Initialize the golf-cup source group's full-circle constant.
void GolfCup_InitFullTurnConstant() {
    s_golfCupFullTurn = kFloat_6_2831855;
}

// Indexed setter contract: movable index in [0,5), cursor index in [0,2).
// No bounds checking. value is a word encoded in a pointer-shaped ABI argument,
// never dereferenced or retained; enabled controls visibility, playerFlag controls
// highlighting. Coordinates use the layout position units, not world coordinates.
// Set one movable group
// Set one movable group's enabled word.
void CInGameTutorialDialog::SetMovableGroupEnabled(int index, void* value) {
    ((TutorialElementGroup*)&elementGroup0X)[index].enabled = (int)value; // original +0x94
}

// Set one cursor group's enabled word.
void CInGameTutorialDialog::SetCursorGroupEnabled(int index, void* value) {
    ((TutorialElementGroup*)&cursorGroup0X)[index].enabled = (int)value; // original +0x74
}

// Set one cursor group's player flag.
void CInGameTutorialDialog::SetCursorGroupPlayerFlag(int index, void* value) {
    ((TutorialElementGroup*)&cursorGroup0X)[index].playerFlag = (int)value; // original +0x70
}

// Set the position of one movable group.
void CInGameTutorialDialog::SetMovableGroupPosition(int index, float x, float y) {
    ((TutorialElementGroup*)&elementGroup0X)[index].x = x; // original +0x88
    ((TutorialElementGroup*)&elementGroup0X)[index].y = y; // original +0x8C
}

// Set the position of one cursor group.
void CInGameTutorialDialog::SetCursorGroupPosition(int index, float x, float y) {
    ((TutorialElementGroup*)&cursorGroup0X)[index].x = x; // original +0x68
    ((TutorialElementGroup*)&cursorGroup0X)[index].y = y; // original +0x6C
}

// Move the mouse cursor to the selected cursor group's position.
void CInGameTutorialDialog::MoveCursorToGroup(int index) {
    TutorialElementGroup* item = (TutorialElementGroup*)&cursorGroup0X + index;
    ((CMouseCursor*)g_mouseCursor)->MoveTo(item->x, item->y, kTutorialCursorMoveSentinel); // original +0x68/+0x6C
}

} // extern C
// Initialize the cup base vtable and publish the preview pointer, then the cup vtable,
// resource slots and two identity matrices, then position and the three-times-scaled preview mesh.
CGolfCup::CGolfCup() {
    BaseObject_ctor(this);
    *(void**)this = s_golfCupBaseVtable;
    g_golfCupPreview = this;
    *(void**)this = __vt__8CGolfCup;
    model = 0;
    captureQueue = 0;
    active = 0;
    shadowMesh = 0;
    captureTexture.ptr = 0;
    PSMTX44Identity(camera);
    PSMTX44Identity(projection);
    register void* self = this;
    float f1 = kGolfCupPreviewMeshScale;
    float f0 = kFloat_0_GolfCup;
    float f2 = f1;
    *(volatile int*)&buffer = 0;
    float f3 = f1;
    *(volatile int*)&size = 0;
    *(volatile int*)&capacity = 0;
    pos.x = f0;
    pos.y = f0;
    pos.z = f0;
    int arg0 = 0;
    int arg1 = 1;
    GolfCupMesh_CreateScaledTriangles(self, (void*)&((volatile CGolfCup*)self)->shadowMesh, arg0, arg1, f1, f2, f3);
}
extern "C" {

struct ActivatedList {
    int* data;
    int count;
    int capacity;
};

// Update the layout and publish button 0/1 as dialog result 1/2 while input is enabled.
void CInGameTutorialDialog::UpdateButtonSelection() {
    if (g_tutorialSharedLayout != 0 && ((DialogV4*)this)->v14() != 0) {
        ((DialogV4*)this)->v80();
        Layout_Update(g_tutorialSharedLayout);
        if (g_tutorialSharedLayoutReady != 0 && dialogState == kDialogAcceptingInput) {
            ActivatedList list = {};
            ActivatedUnwindCleanup activatedCleanup(&list);
            int selected = -1;
            int result = Layout_CollectActivated(g_tutorialSharedLayout, &list, 0);
            if (result != 0) {
                if (list.data[0] == 0)
                    selected = 0;
                else if (list.data[0] == 1)
                    selected = 1;
            }
            if (selected != -1) {
                int points[2];
                points[0] = kTutorialPrimaryButtonResult[0];
                points[1] = kTutorialSecondaryButtonResult;
                selectionResult = points[selected];
                ((DialogV4*)this)->v10(1, points);
            }
            if (list.data != 0) {
                list.count -= ((char*)&list + list.count) - (char*)&list;
                activatedCleanup.release();
                appFreeUni__FPv(list.data);
            }
        }
    }
}

#pragma opt_lifetimes off
// Draw the visible dialog and place its text at the first enabled text anchor.
void CInGameTutorialDialog::DrawDialogWithText() {
    if (g_tutorialSharedLayout != 0 && g_tutorialSharedLayoutReady != 0 && opacity > kTutorialMinimumDrawOpacity) {
        ((DialogV4*)this)->v80();
        Layout_DrawWithOpacity(g_tutorialSharedLayout, opacity);
        if (InGameDialog_GetNonemptyWideText(this) != 0) {
            int* row;
            int index;
            int offset;
            int tutorialPageIndex;
            volatile int* choices;
            choices = (volatile int*)((char*)(s_tutorialLayoutTables.textAnchorElements));
            tutorialPageIndex = tutorialIndex;
            row = s_tutorialElementVisibilityByPage[tutorialPageIndex];
            index = -1;
            offset = choices[0] * 4;
            if (*(int*)(offset + (char*)row) != 0) {
                index = choices[0];
            } else {
                offset = choices[1] * 4;
                if (*(int*)(offset + (char*)row) != 0) {
                    index = choices[1];
                } else {
                    offset = choices[2] * 4;
                    if (*(int*)(offset + (char*)row) != 0) {
                        index = choices[2];
                    } else {
                        offset = choices[3] * 4;
                        if (*(int*)(offset + (char*)row) != 0)
                            index = choices[3];
                    }
                }
            }
            if (index != -1) {
                int point[2];
                Layout_GetElementPosition(g_tutorialSharedLayout, index, point);
                int arg = Dialog_GetOpacityByte(this);
                DialogText_DrawAligned(this, point, arg, 0, 0);
            }
        }
    }
}

#pragma opt_lifetimes reset
// Report whether either of the two tutorial buttons was activated.
int CInGameTutorialDialog::IsEitherButtonActivated() {
    ActivatedList list = {};
    ActivatedUnwindCleanup activatedCleanup(&list);
    int result = Layout_CollectActivated(g_tutorialSharedLayout, &list, 0);
    if (result != 0) {
        int first = list.data[0];
        if (first == 0) {
            if (list.data != 0) {
                list.count -= ((char*)&list + list.count) - (char*)&list;
                activatedCleanup.release();
                appFreeUni__FPv(list.data);
            }
            return 1;
        }
        if (first == 1) {
            if (list.data != 0) {
                list.count -= ((char*)&list + list.count) - (char*)&list;
                activatedCleanup.release();
                appFreeUni__FPv(list.data);
            }
            return 1;
        }
    }
    if (list.data != 0) {
        list.count -= ((char*)&list + list.count) - (char*)&list;
        activatedCleanup.release();
        appFreeUni__FPv(list.data);
    }
    return 0;
}

// Set an element position and rebuild its integer hit rectangle from its layout size.
void CInGameTutorialDialog::SetElementPositionAndRect(void* elementId, float* inputXY) {
    // inputXY is two layout-space floats; elementId is an integer carried by the pointer ABI.
    // The temporary first holds XY, then receives width/height; bounds truncate toward zero.
    if (g_tutorialSharedLayout != 0) {
        float positionThenSize[2];
        gtl_Vec2_Copy(positionThenSize, inputXY);
        Layout_SetElementPosition(g_tutorialSharedLayout, elementId, positionThenSize);
        Layout_GetElementSize(g_tutorialSharedLayout, elementId, positionThenSize);
        int bottom, right, top, left;
        float height, width, y, x;
        x = inputXY[0];
        y = inputXY[1];
        width = positionThenSize[0];
        height = positionThenSize[1];
        left = (int)x;
        top = (int)y;
        right = (int)(x + width);
        bottom = (int)(y + height);
        int bounds[4];
        bounds[0] = left;
        bounds[1] = top;
        bounds[2] = right;
        bounds[3] = bottom;
        Layout_WriteElementRect(g_tutorialSharedLayout, elementId, bounds);
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462320", _ctors_u8026456C_0, TutorialDialog_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462320", _ctors_u8026456C_1, GolfCup_InitFullTurnConstant);
#pragma pop
