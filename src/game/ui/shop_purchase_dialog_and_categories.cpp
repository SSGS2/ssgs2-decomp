#include <decomp/static_init.h>
// Translation unit 0x802DABF4-0x802DC0E8 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/UITextTable.h"
#include "game/singletons.h"
#include "classes/CPurchaseDialog.h"
#include "classes/CProductsCategory.h"
#include "classes/CShopSubMenu.h"
#include "classes/CShopLayoutState.h"

#define AsCShopLayoutState(p) ((CShopLayoutState*)(p))

#define AsCShopSubMenu(p) ((CShopSubMenu*)(p))

#define AsCProductsCategory(p) ((CProductsCategory*)(p))

#define AsCPurchaseDialog(p) ((CPurchaseDialog*)(p))

enum PurchaseResult { kPurchasePending = -1, kPurchaseCancelled = 0, kPurchaseConfirmed = 1 };
enum PurchaseLayout {
    kConfirmButton = 0, kCancelButton = 1, kButtonCaptionBase = 2,
    kSlotRectBase = 7, kPurchaseSlotCount = 8, kSelectionDelayFrames = 3,
    kPlayerInteractionFlag = 5, kPressedElementState = 2,
    kActionCancelPurchase = 0x17,
    kBalanceText = 0x2D, kPriceText = 0x2C, kRemainingBalanceText = 0x2E
};
enum CategoryLayout {
    kCategoryCount = 5, kCategoryElementStride = 3, kCategoryMarkerBase = 16,
    kCategoryRowHeight = 34, kSelectedCategoryGap = 68, kNoCategory = -1
};
// Packed pool; lengths include terminators and match the global string-table research.
struct PurchaseDialogStrings {
    char layout[13], bpFile[13], script[9], numberAtlas[16];
    char closeSound[16], openSound[15], cancelSound[23], clickSound[14], padding;
};
typedef char PurchaseDialogStrings_size[(sizeof(PurchaseDialogStrings) == 0x78) ? 1 : -1];

extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    explicit ActivatedUnwindCleanup(volatile void* vector)
        : words((volatile unsigned int*)vector), active(true) {}
    void release() { active = false; }
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};


extern const float kFloat_6_2831855;
extern float s_productsCategoryRuntimeTau;
extern float s_purchaseDialogRuntimeTau;
extern const float kProductsCategoryOpacity;
extern const float kProductsCategorySelectedGap;
extern const float kProductsCategoryRowSpacing;
extern const float kFloat_1_PurchaseDialog;
extern const float kFloat_0_5_PurchaseDialog;
extern const float kFloat_200_PurchaseDialogSound;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRecord_PurchaseDialog[];
extern signed char s_inputManagerInitialized;
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern int Input_IsActionPressed(void*, int, int);
extern void Layout_WriteElementRect(void*, int, int*);
extern void appFreeUni__FPv(void*);


struct Layout;
extern char __vt__17CProductsCategory[];
extern char __vt__15CPurchaseDialog[];
extern const char s_productsCategoryLayoutPath[];
extern const PurchaseDialogStrings purchaseDialogStrings;
extern const char s_purchaseDialogQuadTemplate[];
extern void* g_soundManager;
// Set how many category rows participate in layout and interaction.
int CProductsCategory_SetCategoryCount(CProductsCategory* category, int count) {
    category->categoryCount = count;
    return 1;
}

// Initialize the category unit's runtime full-circle constant.
void InitProductsCategoryRuntimeTau() {
    s_productsCategoryRuntimeTau = kFloat_6_2831855;
}

// Initialize the purchase-dialog unit's runtime full-circle constant.
void InitPurchaseDialogRuntimeTau() {
    s_purchaseDialogRuntimeTau = kFloat_6_2831855;
}

extern void Layout_Update(void* layout);
extern void Layout_DrawWithOpacity(void* layout, float t);
extern unsigned Layout_PollLoadComplete(void* layout);
extern int Layout_CollectActivated(void* layout, void* indices, void* players);
extern void Layout_SetElementEnabled(void* layout, int index, int value);
extern void Layout_SetElementPlayerFlagsAndState(void* layout, int index, int value);
extern void Layout_SetElementValueState(void* layout, int index, int value);
extern void Layout_GetElementPosition(void* layout, int index, float* out);
extern void Layout_SetElementPosition(void* layout, int index, float* out);
extern void BpObject_TryInvokeCurrentCallback(void* layout, int value);
extern void BpObject_Draw(void* layout);
extern int CProductsCategory_UpdateSelectionLayout(void* layout);
extern int CPurchaseDialog_UpdateSelectionDelay(void* layout);
extern int CPurchaseDialog_ProcessInput(void* layout);
extern int CPurchaseDialog_DrawSlotQuads(void* layout);
extern int CProductsCategory_SetLayoutMode(void* layout, int v);
extern void UITextTable_Draw(void* layout);
extern void UITextTable_SetInteger(void* layout, int key, int count);
extern void Layout_Destroy(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, const char*);
extern void BpObject_SetEnabled(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*);
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetModeByte(void*, int);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void CFontNumDraw_RegisterEntries(void*, void*, int, void*, const char*);
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_PurchaseDialog[];
extern char s_purchaseDialogPriceTextEntries[];
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
extern void* __nw__FUl(unsigned int);
extern void* __register_global_object(void*, void*, void*);
extern void __dl__FPv(void*);
extern char s_fontNumDrawRegistry[];
extern char s_fontNumDrawRegistryDtorRecord_PurchaseDialog[];
extern void PointerVector_DestroyStorage(void*);
extern void UITextTable_ClearEntries(void*);
extern void PointerVector_InitReserved(void*);
extern void PointerArray_DeleteValue(void*, void*);
extern void CBpCtrl_DeleteEntryById(void*, int);
extern void Layout_ReadElementRect(void*, int, int*);
extern void Layout_LoadExtended(void*, int, const char*);
extern void CBpCtrl_CreateEntry(void*, const char*, int*, int);
extern int IdMap_Lookup(void*, int);
extern void* CFontNumDraw_CreateInVector(void*);
extern void Engine_SetFlexibleVertexFormat(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
// Store a drawable handle in one of the purchase dialog's eight slots.
int CPurchaseDialog_SetSlotDrawable(void* p, int idx, int val) {
    if (idx >= kPurchaseSlotCount) {
        return 0;
    }
    int* base = (int*)&AsCPurchaseDialog(p)->slot0Texture;
    base[idx] = val;
    return 1;
}

// Advance the category layout's animation and interaction state.
int CProductsCategory_UpdateLayout(void* p) {
    void* q = AsCProductsCategory(p)->layout;
    Layout_Update(q);
    return 1;
}

// Draw the category layout at its configured opacity.
int CProductsCategory_Draw(void* p) {
    void* q = AsCProductsCategory(p)->layout;
    float t = kProductsCategoryOpacity;
    Layout_DrawWithOpacity(q, t);
    return 1;
}

// Report whether the shop layout has finished loading.
int CShopLayoutState_IsLoaded(void* p) {
    void* q = AsCShopLayoutState(p)->layout;
    return Layout_PollLoadComplete(q) != 0;
}

// Set each row's pressed state and swap its normal/selected captions.
int CProductsCategory_UpdateSelectionFlags(void* p) {
    void* category = p;
    int categoryIndex = 0;
    int elementBase = 0;
    int selectedIndex;
    int buttonElement;
    int normalCaptionElement;
    int buttonState;
    int unselected;
    while (categoryIndex < AsCProductsCategory(category)->categoryCount) {
        selectedIndex = AsCProductsCategory(category)->selectedCategoryIndex;
        buttonElement = elementBase + 1;
        normalCaptionElement = elementBase + 2;
        if (selectedIndex == categoryIndex) {
            buttonState = 2;
            unselected = 0;
        } else {
            buttonState = 0;
            unselected = 1;
        }
        Layout_SetElementPlayerFlag(AsCProductsCategory(category)->layout, buttonElement, unselected, kPlayerInteractionFlag);
        Layout_SetElementValueState(AsCProductsCategory(category)->layout, buttonElement, buttonState);
        Layout_SetElementEnabled(AsCProductsCategory(category)->layout, normalCaptionElement, unselected);
        unsigned showSelectedCaption = (unselected == 0) ? 1 : 0;
        Layout_SetElementEnabled(AsCProductsCategory(category)->layout, elementBase + 3, showSelectedCaption);
        elementBase += kCategoryElementStride;
        categoryIndex++;
    }
    return 1;
}

// Collect category layout activations into the caller's index vector; return
// whether any were collected. This mutates indices even though the result is Boolean.
int CProductsCategory_CollectActivatedIndices(void* p, void* indices) {
    void* q = AsCProductsCategory(p)->layout;
    return Layout_CollectActivated(q, indices, 0) != 0;
}

// Show or hide the auxiliary marker for a category index.
int CProductsCategory_SetMarkerVisible(void* p, int idx, int val) {
    if (idx >= 5) {
        return 0;
    }
    void* q = AsCProductsCategory(p)->layout;
    Layout_SetElementEnabled(q, idx + 0x10, val);
    return 1;
}

// Update the dialog layout and run the current BP callback.
int CPurchaseDialog_UpdateLayoutAndScript(void* p) {
    void* r31 = p;
    void* q = AsCPurchaseDialog(p)->layout;
    Layout_Update(q);
    void* r = (*(void**)&AsCPurchaseDialog(r31)->bpObject);
    BpObject_TryInvokeCurrentCallback(r, 0);
    return 1;
}

// Validate and apply the selected category, then reposition the layout.
int CProductsCategory_SetSelectedIndex(void* p, int idx) {
    // 0x802DB0A8: accept -1 through categoryCount inclusive; preserve the upper-bound convention.
    if (idx < -1 || idx > AsCProductsCategory(p)->categoryCount)
        return 0;
    AsCProductsCategory(p)->selectedCategoryIndex = idx;
    CProductsCategory_UpdateSelectionLayout(p);
    return 1;
}

// Hold pressed feedback, then restore the selected button caption position and clear the delay.
int CPurchaseDialog_UpdateSelectionDelay(void* self) {
    int remainingFrames = AsCPurchaseDialog(self)->selectionDelayFrames;
    if (remainingFrames == -1) {
        return 0;
    }
    int nextFrames = remainingFrames - 1;
    AsCPurchaseDialog(self)->selectionDelayFrames = nextFrames;
    if (remainingFrames > 0) {
        return 1;
    }
    float pressedOffset = kFloat_1_PurchaseDialog;
    int buttonIndex = 0;
    int zero = 0;
    while (1) {
        if (buttonIndex == AsCPurchaseDialog(self)->selectedElement) {
            float captionPosition[2] = {0.0f, 0.0f};
            Layout_GetElementPosition(AsCPurchaseDialog(self)->layout, buttonIndex + kButtonCaptionBase, captionPosition);
            captionPosition[0] -= pressedOffset;
            captionPosition[1] -= pressedOffset;
            Layout_SetElementPosition(AsCPurchaseDialog(self)->layout, buttonIndex + kButtonCaptionBase, captionPosition);
        }
        Layout_SetElementValueState(AsCPurchaseDialog(self)->layout, buttonIndex, 0);
        buttonIndex++;
        if (buttonIndex > kCancelButton) {
            break;
        }
    }
    AsCPurchaseDialog(self)->selectionDelayFrames = kPurchasePending;
    return 1;
}

// Process input when feedback permits, then advance the layout.
int CPurchaseDialog_Update(void* p) {
    void* r31 = p;
    if (CPurchaseDialog_UpdateSelectionDelay(p) == 0) {
        CPurchaseDialog_ProcessInput(p);
    }
    void* q = AsCPurchaseDialog(r31)->layout;
    Layout_Update(q);
    return 1;
}

// Draw the BP background, layout, item-slot quads, and number table.
int CPurchaseDialog_Draw(void* p) {
    void* r31 = p;
    BpObject_TryInvokeCurrentCallback((*(void**)&AsCPurchaseDialog(p)->bpObject), 0);
    BpObject_Draw((*(void**)&AsCPurchaseDialog(r31)->bpObject));
    Layout_DrawWithOpacity(AsCPurchaseDialog(r31)->layout, kFloat_1_PurchaseDialog);
    CPurchaseDialog_DrawSlotQuads(r31);
    UITextTable_Draw(AsCPurchaseDialog(r31)->textTable);
    return 1;
}

// Apply one of three row styles and enable only the configured categories.
int CProductsCategory_SetLayoutMode(void* p, int v) {
    if (v >= 3 || v < 0) {
        return 0;
    }
    int r31 = 0;
    int r30 = 0;
    while (r31 < 3) {
        Layout_SetElementValueState(AsCProductsCategory(p)->layout, r30 + 2, (signed char)v);
        Layout_SetElementValueState(AsCProductsCategory(p)->layout, r30 + 3, (signed char)v);
        r31++;
        r30 += 3;
    }
    v = 0;
    {
        register int r30 = 0;
        while (v < 5) {
            int r0 = AsCProductsCategory(p)->categoryCount;
            register int r4 = r30 + 1;
            register int r31 = r30 + 2;
            if (v < r0) {
                Layout_SetElementPlayerFlagsAndState(AsCProductsCategory(p)->layout, r4, 1);
                Layout_SetElementEnabled(AsCProductsCategory(p)->layout, r31, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(AsCProductsCategory(p)->layout, r4, 0);
                Layout_SetElementEnabled(AsCProductsCategory(p)->layout, r31, 0);
            }
            Layout_SetElementEnabled(AsCProductsCategory(p)->layout, r30 + 3, 0);
            v++;
            r30 += 3;
        }
    }
    return 1;
}

// Apply the category style, then validate and display the requested selection.
int CProductsCategory_SetModeAndSelection(void* p, int v, int idx) {
    void* r30 = p;
    int r31 = idx;
    AsCProductsCategory(p)->selectedCategoryIndex = kNoCategory;
    CProductsCategory_SetLayoutMode(p, v);
    if (r31 >= -1) {
        int max = AsCProductsCategory(r30)->categoryCount;
        if (r31 <= max) {
            AsCProductsCategory(r30)->selectedCategoryIndex = r31;
            CProductsCategory_UpdateSelectionLayout(r30);
        }
    }
    return 1;
}

// Display the current balance, purchase price, and remaining balance.
int CPurchaseDialog_SetPriceBreakdown(void* self, int balance, int price) {
    void* dialog = self;
    int currentBalance = balance;
    int purchasePrice = price;
    UITextTable_SetInteger(AsCPurchaseDialog(self)->textTable, kBalanceText, currentBalance);
    UITextTable_SetInteger(AsCPurchaseDialog(dialog)->textTable, kPriceText, purchasePrice);
    UITextTable_SetInteger(AsCPurchaseDialog(dialog)->textTable, kRemainingBalanceText, currentBalance - purchasePrice);
    return 1;
}

struct Obj77DB4 { char pad[0x5c]; Obj77DB4(int n) { Layout_Init(this, n); } };

// Create the dialog layout, BP resource entry, number table, and empty drawable slots.
void* CPurchaseDialog_Ctor(void* self_) {
    register char* self = (char*)self_;
    AsCPurchaseDialog(self)->unk_4 = 0;
    *(void**)self = __vt__15CPurchaseDialog;
    void* q = new Obj77DB4(5);
    AsCPurchaseDialog(self)->layout = q;
    AsCPurchaseDialog(self)->bpObject = 0;
    AsCPurchaseDialog(self)->bpEntryId = -1;
    AsCPurchaseDialog(self)->result = kPurchasePending;
    (*(int*)&AsCPurchaseDialog(self)->textTable) = 0;
    AsCPurchaseDialog(self)->selectionDelayFrames = kPurchasePending;
    AsCPurchaseDialog(self)->selectedElement = -1;
    AsCPurchaseDialog(self)->slot0Texture = 0;
    AsCPurchaseDialog(self)->slot1Texture = 0;
    AsCPurchaseDialog(self)->slot2Texture = 0;
    AsCPurchaseDialog(self)->slot3Texture = 0;
    AsCPurchaseDialog(self)->slot4Texture = 0;
    AsCPurchaseDialog(self)->slot5Texture = 0;
    AsCPurchaseDialog(self)->slot6Texture = 0;
    AsCPurchaseDialog(self)->slot7Texture = 0;
    Layout_LoadExtended(q, 0, purchaseDialogStrings.layout);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), purchaseDialogStrings.bpFile, (int*)(self + (int)&((CPurchaseDialog*)0)->bpEntryId), 0);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    int h = IdMap_Lookup(Sing_805DC5F0(), AsCPurchaseDialog(self)->bpEntryId);
    AsCPurchaseDialog(self)->bpObject = h;
    if (Sing_805DC830() == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (void*)s_fontNumDrawRegistryDtorRecord_PurchaseDialog);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    AsCPurchaseDialog(self)->textTable = CFontNumDraw_CreateInVector(Sing_805DC830());
    return self;
}

struct SlotDrawable {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void draw();
    virtual int visible();
};
struct SlotVertices { unsigned words[24]; };

// Draw each visible slot drawable into its layout rectangle as a textured quad.
int CPurchaseDialog_DrawSlotQuads(void* self) {
    unsigned* quadTemplate = (unsigned*)s_purchaseDialogQuadTemplate;
    void* slotCursor = self;
    int index = 0;
    int zero = 0;
    for (;;) {
        if (*(void**)&AsCPurchaseDialog(slotCursor)->slot0Texture == 0) break;
        if ((*(SlotDrawable**)&AsCPurchaseDialog(slotCursor)->slot0Texture)->visible() != 0) {
            SlotVertices vertices;
            int rectangle[4];
            rectangle[0] = zero;
            rectangle[1] = zero;
            rectangle[2] = zero;
            rectangle[3] = zero;
            Layout_ReadElementRect(AsCPurchaseDialog(self)->layout, index + kSlotRectBase, rectangle);
            vertices = *(SlotVertices*)quadTemplate;
            float* vertexWords = (float*)&vertices;
            vertexWords[0] = (float)rectangle[0];
            vertexWords[1] = (float)rectangle[1];
            vertexWords[6] = (float)rectangle[2];
            vertexWords[7] = (float)rectangle[1];
            vertexWords[12] = (float)rectangle[0];
            vertexWords[13] = (float)rectangle[3];
            vertexWords[18] = (float)rectangle[2];
            vertexWords[19] = (float)rectangle[3];
            Engine_SetFlexibleVertexFormat(0x11F005);
            (*(SlotDrawable**)&AsCPurchaseDialog(slotCursor)->slot0Texture)->draw();
            void* primitiveCount = GX_GetPrimitiveCount(4, 0x98);
            DrawPrimitiveUP(0x98, primitiveCount, &vertices, 0x18);
        }
        ++index;
        slotCursor = (char*)slotCursor + 4;
        if (index >= kPurchaseSlotCount) break;
    }
    return 1;
}

// Destroy the category layout and optionally delete its owning object.
void* CProductsCategory_dtor(void* p, int deleting) {
    if (p != 0) {
        AsCProductsCategory(p)->vtbl = __vt__17CProductsCategory;
        void* q = AsCProductsCategory(p)->layout;
        if (q != 0) {
            Layout_Destroy(q, 1);
            AsCProductsCategory(p)->layout = 0;
        }
        if (deleting > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Create the category layout and initialize its row count and selected index.
void* CProductsCategory_Ctor(void* p, int arg) {
    AsCProductsCategory(p)->layout = 0;
    AsCProductsCategory(p)->vtbl = __vt__17CProductsCategory;
    AsCProductsCategory(p)->categoryCount = arg;
    AsCProductsCategory(p)->selectedCategoryIndex = 0;
    void* r31 = new Obj77DB4(5);
    AsCProductsCategory(p)->layout = r31;
    Layout_LoadExtended(r31, 0, s_productsCategoryLayoutPath);
    return p;
}

// Publish a settled purchase result, hide the dialog, and play its close sound.
int CPurchaseDialog_TryCloseWithResult(void* self, void* out) {
    int result = AsCPurchaseDialog(self)->result;
    if (result == kPurchasePending) {
        return 0;
    }
    int feedbackFrames = AsCPurchaseDialog(self)->selectionDelayFrames;
    if (feedbackFrames != kPurchasePending) {
        return 0;
    }
    *(int*)out = result;
    BpObject_SetEnabled((*(void**)&AsCPurchaseDialog(self)->bpObject), 0);
    void* textTable = AsCPurchaseDialog(self)->textTable;
    AsUITextTable(textTable)->drawEnabled = 0; // original text table +0x28
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.closeSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog, kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    return 1;
}

// Wait for both the submenu layout and shared BP entries to finish loading.
int CShopSubMenu_IsLoaded(void* p) {
    void* q = AsCShopSubMenu(p)->layout;
    if (Layout_PollLoadComplete(q) == 0) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) == 0) {
        return 0;
    }
    return 1;
}

// Reset the result, enable the dialog controls and text, and play the open sound.
int CPurchaseDialog_Show(void* self) {
    void* r31 = self;
    AsCPurchaseDialog(self)->result = kPurchasePending;
    BpObject_SetEnabled((*(void**)&AsCPurchaseDialog(self)->bpObject), 1);
    void* r3 = AsCPurchaseDialog(r31)->textTable;
    AsUITextTable(r3)->drawEnabled = 1; // original text table +0x28
    Layout_SetElementPlayerFlag(AsCPurchaseDialog(r31)->layout, 0, 1, kPlayerInteractionFlag);
    Layout_SetElementPlayerFlag(AsCPurchaseDialog(r31)->layout, 1, 1, kPlayerInteractionFlag);
    Layout_Update(AsCPurchaseDialog(r31)->layout);
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.openSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog, kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    return 1;
}

// Start the purchase BP script, hide item panes, and register price text entries.
int CPurchaseDialog_InitLayout(void* self) {
    void* r30 = self;
    Layout_SetModeByte(AsCPurchaseDialog(self)->layout, 0);
    BpObject_StartScriptByName((*(void**)&AsCPurchaseDialog(self)->bpObject), purchaseDialogStrings.script, 1);
    BpObject_SetEnabled((*(void**)&AsCPurchaseDialog(self)->bpObject), 0);
    int r31 = 4;
    do {
        Layout_SetElementEnabled(AsCPurchaseDialog(r30)->layout, r31, 0);
        r31++;
    } while (r31 <= 0xe);
    CFontNumDraw_RegisterEntries(AsCPurchaseDialog(r30)->textTable, s_purchaseDialogPriceTextEntries, 3, (*(void**)&AsCPurchaseDialog(r30)->bpObject), purchaseDialogStrings.numberAtlas);
    void* r4 = AsCPurchaseDialog(r30)->textTable;
    AsUITextTable(r4)->drawEnabled = 1; // original text table +0x28
    return 1;
}

// Release the layout and BP entry, unregister the text table, and optionally delete the dialog.
void* CPurchaseDialog_dtor(void* self, int deleting) {
    void* r30 = self;
    if (self != 0) {
        *(void**)self = __vt__15CPurchaseDialog;
        void* q = AsCPurchaseDialog(self)->layout;
        if (q != 0) {
            Layout_Destroy(q, 1);
            AsCPurchaseDialog(r30)->layout = 0;
        }
        if (AsCPurchaseDialog(r30)->bpEntryId != -1) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCPurchaseDialog(r30)->bpEntryId);
            AsCPurchaseDialog(r30)->bpEntryId = -1;
        }
        UITextTable_ClearEntries(AsCPurchaseDialog(r30)->textTable);
        if (AsCPurchaseDialog(r30)->textTable != 0) {
            if (Sing_805DC830() == 0) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (void*)s_fontNumDrawRegistryDtorRecord_PurchaseDialog);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCPurchaseDialog(r30)->textTable);
        }
        if (deleting > 0) __dl__FPv(r30);
    }
    return r30;
}

// Calculate selection-marker positions, hide auxiliary rows, and apply the initial mode.
int CProductsCategory_InitLayout(void* self, int mode, int selected) {
    float anchorPosition[2];
    int zero = 0;
    int secondRowOffset = kCategoryRowHeight;
    int thirdRowOffset = 2 * kCategoryRowHeight;
    int fourthRowOffset = 3 * kCategoryRowHeight;
    int fifthRowOffset = 4 * kCategoryRowHeight;
    Layout_SetModeByte(AsCProductsCategory(self)->layout, 0);
    ((int*)anchorPosition)[0] = zero;
    ((int*)anchorPosition)[1] = zero;
    Layout_GetElementPosition(AsCProductsCategory(self)->layout, 0, anchorPosition);
    AsCProductsCategory(self)->selectionY[0] = (int)(anchorPosition[1] + (float)zero);
    AsCProductsCategory(self)->selectionY[1] = (int)(anchorPosition[1] + (float)secondRowOffset);
    AsCProductsCategory(self)->selectionY[2] = (int)(anchorPosition[1] + (float)thirdRowOffset);
    AsCProductsCategory(self)->selectionY[3] = (int)(anchorPosition[1] + (float)fourthRowOffset);
    AsCProductsCategory(self)->selectionY[4] = (int)(anchorPosition[1] + (float)fifthRowOffset);
    for (int i = kCategoryMarkerBase; i <= kCategoryMarkerBase + kCategoryCount - 1; ++i) {
        Layout_SetElementEnabled(AsCProductsCategory(self)->layout, i, 0);
    }
    AsCProductsCategory(self)->selectedCategoryIndex = kNoCategory;
    CProductsCategory_SetLayoutMode(self, mode);
    if (selected >= -1 && selected <= AsCProductsCategory(self)->categoryCount) {
        AsCProductsCategory(self)->selectedCategoryIndex = selected;
        CProductsCategory_UpdateSelectionLayout(self);
    }
    return 1;
}

// Space category rows around the selected item and move the selection marker.
int CProductsCategory_UpdateSelectionLayout(void* self) {
    float buttonPosition[2];
    float captionPosition[2];
    float markerPosition[2];
    int buttonRect[4];
    Layout_GetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), 1, buttonPosition);
    Layout_GetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), 2, captionPosition);
    Layout_GetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), kCategoryMarkerBase, markerPosition);
    Layout_ReadElementRect((*(void* volatile*)&AsCProductsCategory(self)->layout), 1, buttonRect);
    float selectedGap = kProductsCategorySelectedGap;
    int categoryIndex = 1;
    float rowHeight = kProductsCategoryRowSpacing;
    int elementBase = 3;
    while (categoryIndex < kCategoryCount) {
        int selectedIndex = AsCProductsCategory(self)->selectedCategoryIndex;
        int buttonElement = elementBase + 1;
        int captionElement = elementBase + 2;
        if (categoryIndex == selectedIndex + 1) {
            buttonPosition[1] += selectedGap;
            captionPosition[1] += selectedGap;
            markerPosition[1] += selectedGap;
            buttonRect[1] += kSelectedCategoryGap;
            buttonRect[3] += kSelectedCategoryGap;
        }
        buttonPosition[1] += rowHeight;
        Layout_SetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), buttonElement, buttonPosition);
        captionPosition[1] += rowHeight;
        Layout_SetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), captionElement, captionPosition);
        Layout_SetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), elementBase + 3, captionPosition);
        markerPosition[1] += rowHeight;
        Layout_SetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), categoryIndex + kCategoryMarkerBase, markerPosition);
        buttonRect[1] += kCategoryRowHeight;
        buttonRect[3] += kCategoryRowHeight;
        Layout_WriteElementRect((*(void* volatile*)&AsCProductsCategory(self)->layout), buttonElement, buttonRect);
        categoryIndex++;
        elementBase += kCategoryElementStride;
    }
    CProductsCategory_UpdateSelectionFlags(self);
    float position[2] = {0.0f, 0.0f};
    int showSelection = 0;
    if (AsCProductsCategory(self)->selectedCategoryIndex != -1) showSelection = 1;
    Layout_SetElementEnabled((*(void* volatile*)&AsCProductsCategory(self)->layout), 0, showSelection);
    if (AsCProductsCategory(self)->selectedCategoryIndex != -1) {
        Layout_GetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), 0, position);
        int idx = AsCProductsCategory(self)->selectedCategoryIndex;
        int selectedY = AsCProductsCategory(self)->selectionY[idx];
        float selectedPositionY = (float)(double)selectedY;
        position[1] = selectedPositionY;
        Layout_SetElementPosition((*(void* volatile*)&AsCProductsCategory(self)->layout), 0, position);
    }
    return 1;
}

// Handle cancel or a confirm/cancel button activation and begin pressed feedback.
int CPurchaseDialog_ProcessInput(void* self) {
    int captionPosition[2];
    // Raw vector header: data address, element count, capacity; IDs are in data.
    int activatedElementVectorWords[3];
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, (void*)s_inputManagerDtorRecord_PurchaseDialog);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = (void*)s_gameInputManager;
    }
    int cancelPressed = Input_IsActionPressed(Sing_805DC5C8(), kActionCancelPurchase, -1);
    if (cancelPressed != 0) {
        AsCPurchaseDialog(self)->result = kPurchaseCancelled;
        Sound_PlayByName(g_soundManager, purchaseDialogStrings.cancelSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog, kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
        return 1;
    }
    register int zero = 0;
    activatedElementVectorWords[0] = zero; activatedElementVectorWords[1] = zero; activatedElementVectorWords[2] = zero;
    ActivatedUnwindCleanup activatedCleanup(&activatedElementVectorWords);
    if (Layout_CollectActivated(AsCPurchaseDialog(self)->layout, activatedElementVectorWords, 0) == 0) {
        if (activatedElementVectorWords[0] != 0) {
            activatedElementVectorWords[1] = activatedElementVectorWords[1] - *(int*)&activatedElementVectorWords[1];
            activatedCleanup.release(); appFreeUni__FPv((void*)activatedElementVectorWords[0]);
        }
        return 0;
    }
    int selectedButton = -1;
    for (unsigned i = 0; i < (unsigned)activatedElementVectorWords[1]; ++i) {
        int elementId = ((int*)activatedElementVectorWords[0])[i];
        if (elementId >= kConfirmButton && elementId <= kCancelButton) {
            selectedButton = elementId;
            break;
        }
    }
    if (selectedButton == -1) {
        if (activatedElementVectorWords[0] != 0) {
            activatedElementVectorWords[1] = activatedElementVectorWords[1] - *(int*)&activatedElementVectorWords[1];
            activatedCleanup.release(); appFreeUni__FPv((void*)activatedElementVectorWords[0]);
        }
        return 0;
    }
    AsCPurchaseDialog(self)->selectedElement = selectedButton;
    Layout_SetElementValueState(AsCPurchaseDialog(self)->layout, selectedButton, kPressedElementState);
    Layout_SetElementPlayerFlag(AsCPurchaseDialog(self)->layout, AsCPurchaseDialog(self)->selectedElement, 0, kPlayerInteractionFlag);
    captionPosition[0] = 0; captionPosition[1] = 0;
    Layout_GetElementPosition(AsCPurchaseDialog(self)->layout, AsCPurchaseDialog(self)->selectedElement + kButtonCaptionBase, (float*)captionPosition);
    *(float*)&captionPosition[0] += kFloat_1_PurchaseDialog;
    *(float*)&captionPosition[1] += kFloat_1_PurchaseDialog;
    Layout_SetElementPosition(AsCPurchaseDialog(self)->layout, AsCPurchaseDialog(self)->selectedElement + kButtonCaptionBase, (float*)captionPosition);
    AsCPurchaseDialog(self)->selectionDelayFrames = kSelectionDelayFrames;
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.clickSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog, kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    if (selectedButton == kConfirmButton) {
        AsCPurchaseDialog(self)->result = kPurchaseConfirmed;
    } else {
        AsCPurchaseDialog(self)->result = kPurchaseCancelled;
    }
    if (activatedElementVectorWords[0] != 0) {
        activatedElementVectorWords[1] = activatedElementVectorWords[1] - *(int*)&activatedElementVectorWords[1];
        activatedCleanup.release(); appFreeUni__FPv((void*)activatedElementVectorWords[0]);
    }
    return 1;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046254C", _ctors_u802DABF4_0, InitProductsCategoryRuntimeTau);
STATIC_INIT_ENTRY(".CRT$XCU8046254C", _ctors_u802DABF4_1, InitPurchaseDialogRuntimeTau);
#pragma pop
