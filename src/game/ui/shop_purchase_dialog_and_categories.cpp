// Shop purchase dialog and the product category rows.
//
// CPurchaseDialog shows the balance, price and remaining balance, and handles its confirm and
// cancel buttons. CProductsCategory lays out and selects the category rows.

#include <decomp/static_init.h>

#include "classes/UITextTable.h"
#include "game/singletons.h"
#include "classes/CPurchaseDialog.h"
#include "classes/CProductsCategory.h"
#include "classes/CShopSubMenu.h"
#include "classes/CShopLayoutState.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"

#define AsCProductsCategory(p) ((CProductsCategory*)(p))

#define AsCPurchaseDialog(p) ((CPurchaseDialog*)(p))

enum PurchaseResult {
    kPurchasePending = -1,
    kPurchaseCancelled = 0,
    kPurchaseConfirmed = 1
};
enum PurchaseLayout {
    kConfirmButton = 0,
    kCancelButton = 1,
    kButtonCaptionBase = 2,
    kSlotRectBase = 7,
    kPurchaseSlotCount = 8,
    kSelectionDelayFrames = 3,
    kPlayerInteractionFlag = 5,
    kPressedElementState = 2,
    kActionCancelPurchase = 0x17,
    kBalanceText = 0x2D,
    kPriceText = 0x2C,
    kRemainingBalanceText = 0x2E
};
enum CategoryLayout {
    kCategoryCount = 5,
    kCategoryElementStride = 3,
    kCategoryMarkerBase = 16,
    kCategoryRowHeight = 34,
    kSelectedCategoryGap = 68,
    kNoCategory = -1
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
    explicit ActivatedUnwindCleanup(volatile void* vector) : words((volatile unsigned int*)vector), active(true) {}
    void release() {
        active = false;
    }
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
int CProductsCategory::SetCategoryCount(int count) {
    categoryCount = count;
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
extern void UITextTable_Draw(void* layout);
extern void UITextTable_SetInteger(void* layout, int key, int count);
extern void Layout_Destroy(void*, int);
extern void Layout_Init(void*, int);
extern void Layout_LoadExtended(void*, int, const char*);
extern void BpObject_SetEnabled(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void CBpCtrl_dtor(void*);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetModeByte(void*, int);
extern void BpObject_StartScriptByName(void*, const char*, int);
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
extern void Layout_ReadElementRect(void*, int, int*);
extern void Layout_LoadExtended(void*, int, const char*);
extern int IdMap_Lookup(void*, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
// Store a drawable handle in one of the purchase dialog's eight slots.
int CPurchaseDialog::SetSlotDrawable(int idx, int val) {
    if (idx >= kPurchaseSlotCount) {
        return 0;
    }
    int* base = (int*)&slot0Texture;
    base[idx] = val;
    return 1;
}

// Advance the category layout's animation and interaction state.
int CProductsCategory::UpdateLayout() {
    void* q = layout;
    Layout_Update(q);
    return 1;
}

// Draw the category layout at its configured opacity.
int CProductsCategory::Draw() {
    void* q = layout;
    float t = kProductsCategoryOpacity;
    Layout_DrawWithOpacity(q, t);
    return 1;
}

// Report whether the shop layout has finished loading.
int CShopLayoutState::IsLoaded() {
    void* q = layout;
    return Layout_PollLoadComplete(q) != 0;
}

// Set each row's pressed state and swap its normal/selected captions.
int CProductsCategory::UpdateSelectionFlags(void* p) {
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
        Layout_SetElementPlayerFlag(AsCProductsCategory(category)->layout, buttonElement, unselected,
                                    kPlayerInteractionFlag);
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
// The caller passes a second argument that this function never reads.
int CProductsCategory::CollectActivatedIndices(void* indices, int unused) {
    void* q = layout;
    return Layout_CollectActivated(q, indices, 0) != 0;
}

// Show or hide the auxiliary marker for a category index.
int CProductsCategory::SetMarkerVisible(int idx, int val) {
    if (idx >= 5) {
        return 0;
    }
    void* q = layout;
    Layout_SetElementEnabled(q, idx + 0x10, val);
    return 1;
}

// Update the dialog layout and run the current BP callback.
int CPurchaseDialog::UpdateLayoutAndScript() {
    void* dialog = this;
    void* q = layout;
    Layout_Update(q);
    void* r = (*(void**)&AsCPurchaseDialog(dialog)->bpObject);
    BpObject_TryInvokeCurrentCallback(r, 0);
    return 1;
}

// Validate and apply the selected category, then reposition the layout.
int CProductsCategory::SetSelectedIndex(int idx) {
    // 0x802DB0A8: accept -1 through categoryCount inclusive; preserve the upper-bound convention.
    if (idx < -1 || idx > categoryCount)
        return 0;
    selectedCategoryIndex = idx;
    UpdateSelectionLayout();
    return 1;
}

// Hold pressed feedback, then restore the selected button caption position and clear the delay.
int CPurchaseDialog::UpdateSelectionDelay() {
    int remainingFrames = selectionDelayFrames;
    if (remainingFrames == -1) {
        return 0;
    }
    int nextFrames = remainingFrames - 1;
    selectionDelayFrames = nextFrames;
    if (remainingFrames > 0) {
        return 1;
    }
    float pressedOffset = kFloat_1_PurchaseDialog;
    int buttonIndex = 0;
    int zero = 0;
    while (1) {
        if (buttonIndex == selectedElement) {
            float captionPosition[2] = {0.0f, 0.0f};
            Layout_GetElementPosition(layout, buttonIndex + kButtonCaptionBase, captionPosition);
            captionPosition[0] -= pressedOffset;
            captionPosition[1] -= pressedOffset;
            Layout_SetElementPosition(layout, buttonIndex + kButtonCaptionBase, captionPosition);
        }
        Layout_SetElementValueState(layout, buttonIndex, 0);
        buttonIndex++;
        if (buttonIndex > kCancelButton) {
            break;
        }
    }
    selectionDelayFrames = kPurchasePending;
    return 1;
}

// Process input when feedback permits, then advance the layout.
int CPurchaseDialog::Update(void* p) {
    void* dialog = p;
    if (((CPurchaseDialog*)p)->UpdateSelectionDelay() == 0) {
        ((CPurchaseDialog*)p)->ProcessInput();
    }
    void* q = AsCPurchaseDialog(dialog)->layout;
    Layout_Update(q);
    return 1;
}

// Draw the BP background, layout, item-slot quads, and number table.
int CPurchaseDialog::Draw() {
    void* dialog = this;
    BpObject_TryInvokeCurrentCallback((*(void**)&bpObject), 0);
    BpObject_Draw((*(void**)&AsCPurchaseDialog(dialog)->bpObject));
    Layout_DrawWithOpacity(AsCPurchaseDialog(dialog)->layout, kFloat_1_PurchaseDialog);
    ((CPurchaseDialog*)dialog)->DrawSlotQuads();
    UITextTable_Draw(AsCPurchaseDialog(dialog)->textTable);
    return 1;
}

// Apply one of three row styles and enable only the configured categories.
int CProductsCategory::SetLayoutMode(int v) {
    if (v >= 3 || v < 0) {
        return 0;
    }
    int r31 = 0;
    int r30 = 0;
    while (r31 < 3) {
        Layout_SetElementValueState(layout, r30 + 2, (signed char)v);
        Layout_SetElementValueState(layout, r30 + 3, (signed char)v);
        r31++;
        r30 += 3;
    }
    v = 0;
    {
        register int r30 = 0;
        while (v < 5) {
            int r0 = categoryCount;
            register int r4 = r30 + 1;
            register int r31 = r30 + 2;
            if (v < r0) {
                Layout_SetElementPlayerFlagsAndState(layout, r4, 1);
                Layout_SetElementEnabled(layout, r31, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(layout, r4, 0);
                Layout_SetElementEnabled(layout, r31, 0);
            }
            Layout_SetElementEnabled(layout, r30 + 3, 0);
            v++;
            r30 += 3;
        }
    }
    return 1;
}

// Apply the category style, then validate and display the requested selection.
int CProductsCategory::SetModeAndSelection(int v, int idx) {
    void* category = this;
    int requestedIndex = idx;
    selectedCategoryIndex = kNoCategory;
    SetLayoutMode(v);
    if (requestedIndex >= -1) {
        int max = AsCProductsCategory(category)->categoryCount;
        if (requestedIndex <= max) {
            AsCProductsCategory(category)->selectedCategoryIndex = requestedIndex;
            ((CProductsCategory*)category)->UpdateSelectionLayout();
        }
    }
    return 1;
}

// Display the current balance, purchase price, and remaining balance.
int CPurchaseDialog::SetPriceBreakdown(int balance, int price) {
    void* dialog = this;
    int currentBalance = balance;
    int purchasePrice = price;
    UITextTable_SetInteger(textTable, kBalanceText, currentBalance);
    UITextTable_SetInteger(AsCPurchaseDialog(dialog)->textTable, kPriceText, purchasePrice);
    UITextTable_SetInteger(AsCPurchaseDialog(dialog)->textTable, kRemainingBalanceText, currentBalance - purchasePrice);
    return 1;
}

struct Obj77DB4 {
    char pad[0x5c];
    Obj77DB4(int n) {
        Layout_Init(this, n);
    }
};

// Create the dialog layout, BP resource entry, number table, and empty drawable slots.
// A named method rather than a constructor: a constructor call through placement new adds a null check at the call site.
void* CPurchaseDialog::Init() {
    unk_4 = 0;
    vtbl = __vt__15CPurchaseDialog;
    void* q = new Obj77DB4(5);
    layout = q;
    bpObject = 0;
    bpEntryId = -1;
    result = kPurchasePending;
    (*(int*)&textTable) = 0;
    selectionDelayFrames = kPurchasePending;
    selectedElement = -1;
    slot0Texture = 0;
    slot1Texture = 0;
    slot2Texture = 0;
    slot3Texture = 0;
    slot4Texture = 0;
    slot5Texture = 0;
    slot6Texture = 0;
    slot7Texture = 0;
    Layout_LoadExtended(q, 0, purchaseDialogStrings.layout);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(purchaseDialogStrings.bpFile, &bpEntryId, 0);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    int h = IdMap_Lookup(Sing_805DC5F0(), bpEntryId);
    bpObject = h;
    if (Sing_805DC830() == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     (void*)s_fontNumDrawRegistryDtorRecord_PurchaseDialog);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    textTable = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    return this;
}

struct SlotDrawable {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void draw();
    virtual int visible();
};
struct SlotVertices {
    unsigned words[24];
};

// Draw each visible slot drawable into its layout rectangle as a textured quad.
int CPurchaseDialog::DrawSlotQuads() {
    unsigned* quadTemplate = (unsigned*)s_purchaseDialogQuadTemplate;
    void* slotCursor = this;
    int index = 0;
    int zero = 0;
    for (;;) {
        if (*(void**)&AsCPurchaseDialog(slotCursor)->slot0Texture == 0)
            break;
        if ((*(SlotDrawable**)&AsCPurchaseDialog(slotCursor)->slot0Texture)->visible() != 0) {
            SlotVertices vertices;
            int rectangle[4];
            rectangle[0] = zero;
            rectangle[1] = zero;
            rectangle[2] = zero;
            rectangle[3] = zero;
            Layout_ReadElementRect(layout, index + kSlotRectBase, rectangle);
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
        if (index >= kPurchaseSlotCount)
            break;
    }
    return 1;
}

// Destroy the category layout and optionally delete its owning object.
CProductsCategory::~CProductsCategory() {
    vtbl = __vt__17CProductsCategory;
    void* q = layout;
    if (q != 0) {
        Layout_Destroy(q, 1);
        layout = 0;
    }
}

// Create the category layout and initialize its row count and selected index.
CProductsCategory::CProductsCategory(int arg) {
    layout = 0;
    vtbl = __vt__17CProductsCategory;
    categoryCount = arg;
    selectedCategoryIndex = 0;
    void* r31 = new Obj77DB4(5);
    layout = r31;
    Layout_LoadExtended(r31, 0, s_productsCategoryLayoutPath);
}

// Publish a settled purchase result, hide the dialog, and play its close sound.
int CPurchaseDialog::TryCloseWithResult(void* out) {
    int result = this->result;
    if (result == kPurchasePending) {
        return 0;
    }
    int feedbackFrames = selectionDelayFrames;
    if (feedbackFrames != kPurchasePending) {
        return 0;
    }
    *(int*)out = result;
    BpObject_SetEnabled((*(void**)&bpObject), 0);
    void* textTable = this->textTable;
    AsUITextTable(textTable)->drawEnabled = 0; // original text table +0x28
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.closeSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog,
                     kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    return 1;
}

// Wait for both the submenu layout and shared BP entries to finish loading.
int CShopSubMenu::IsLoaded() {
    void* q = layout;
    if (Layout_PollLoadComplete(q) == 0) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0) {
        return 0;
    }
    return 1;
}

// Reset the result, enable the dialog controls and text, and play the open sound.
int CPurchaseDialog::Show() {
    void* dialog = this;
    result = kPurchasePending;
    BpObject_SetEnabled((*(void**)&bpObject), 1);
    void* r3 = AsCPurchaseDialog(dialog)->textTable;
    AsUITextTable(r3)->drawEnabled = 1; // original text table +0x28
    Layout_SetElementPlayerFlag(AsCPurchaseDialog(dialog)->layout, 0, 1, kPlayerInteractionFlag);
    Layout_SetElementPlayerFlag(AsCPurchaseDialog(dialog)->layout, 1, 1, kPlayerInteractionFlag);
    Layout_Update(AsCPurchaseDialog(dialog)->layout);
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.openSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog,
                     kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    return 1;
}

// Start the purchase BP script, hide item panes, and register price text entries.
int CPurchaseDialog::InitLayout() {
    void* dialog = this;
    Layout_SetModeByte(layout, 0);
    BpObject_StartScriptByName((*(void**)&bpObject), purchaseDialogStrings.script, 1);
    BpObject_SetEnabled((*(void**)&bpObject), 0);
    int r31 = 4;
    do {
        Layout_SetElementEnabled(AsCPurchaseDialog(dialog)->layout, r31, 0);
        r31++;
    } while (r31 <= 0xe);
    ((CFontNumDraw*)(AsCPurchaseDialog(dialog)->textTable))
        ->RegisterEntries(s_purchaseDialogPriceTextEntries, 3, (*(void**)&AsCPurchaseDialog(dialog)->bpObject),
                          purchaseDialogStrings.numberAtlas);
    void* r4 = AsCPurchaseDialog(dialog)->textTable;
    AsUITextTable(r4)->drawEnabled = 1; // original text table +0x28
    return 1;
}

// Release the layout and BP entry, unregister the text table, and optionally delete the dialog.
CPurchaseDialog::~CPurchaseDialog() {
    vtbl = __vt__15CPurchaseDialog;
    void* q = layout;
    if (q != 0) {
        Layout_Destroy(q, 1);
        layout = 0;
    }
    if (bpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PurchaseDialog);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = -1;
    }
    UITextTable_ClearEntries(textTable);
    if (textTable != 0) {
        if (Sing_805DC830() == 0) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                         (void*)s_fontNumDrawRegistryDtorRecord_PurchaseDialog);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), textTable);
    }
}

// Calculate selection-marker positions, hide auxiliary rows, and apply the initial mode.
int CProductsCategory::InitLayout(int mode, int selected) {
    float anchorPosition[2];
    int zero = 0;
    int secondRowOffset = kCategoryRowHeight;
    int thirdRowOffset = 2 * kCategoryRowHeight;
    int fourthRowOffset = 3 * kCategoryRowHeight;
    int fifthRowOffset = 4 * kCategoryRowHeight;
    Layout_SetModeByte(layout, 0);
    ((int*)anchorPosition)[0] = zero;
    ((int*)anchorPosition)[1] = zero;
    Layout_GetElementPosition(layout, 0, anchorPosition);
    selectionY[0] = (int)(anchorPosition[1] + (float)zero);
    selectionY[1] = (int)(anchorPosition[1] + (float)secondRowOffset);
    selectionY[2] = (int)(anchorPosition[1] + (float)thirdRowOffset);
    selectionY[3] = (int)(anchorPosition[1] + (float)fourthRowOffset);
    selectionY[4] = (int)(anchorPosition[1] + (float)fifthRowOffset);
    for (int i = kCategoryMarkerBase; i <= kCategoryMarkerBase + kCategoryCount - 1; ++i) {
        Layout_SetElementEnabled(layout, i, 0);
    }
    selectedCategoryIndex = kNoCategory;
    SetLayoutMode(mode);
    if (selected >= -1 && selected <= categoryCount) {
        selectedCategoryIndex = selected;
        UpdateSelectionLayout();
    }
    return 1;
}

// Space category rows around the selected item and move the selection marker.
int CProductsCategory::UpdateSelectionLayout() {
    float buttonPosition[2];
    float captionPosition[2];
    float markerPosition[2];
    int buttonRect[4];
    Layout_GetElementPosition((*(void* volatile*)&layout), 1, buttonPosition);
    Layout_GetElementPosition((*(void* volatile*)&layout), 2, captionPosition);
    Layout_GetElementPosition((*(void* volatile*)&layout), kCategoryMarkerBase, markerPosition);
    Layout_ReadElementRect((*(void* volatile*)&layout), 1, buttonRect);
    float selectedGap = kProductsCategorySelectedGap;
    int categoryIndex = 1;
    float rowHeight = kProductsCategoryRowSpacing;
    int elementBase = 3;
    while (categoryIndex < kCategoryCount) {
        int selectedIndex = selectedCategoryIndex;
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
        Layout_SetElementPosition((*(void* volatile*)&layout), buttonElement, buttonPosition);
        captionPosition[1] += rowHeight;
        Layout_SetElementPosition((*(void* volatile*)&layout), captionElement, captionPosition);
        Layout_SetElementPosition((*(void* volatile*)&layout), elementBase + 3, captionPosition);
        markerPosition[1] += rowHeight;
        Layout_SetElementPosition((*(void* volatile*)&layout), categoryIndex + kCategoryMarkerBase, markerPosition);
        buttonRect[1] += kCategoryRowHeight;
        buttonRect[3] += kCategoryRowHeight;
        Layout_WriteElementRect((*(void* volatile*)&layout), buttonElement, buttonRect);
        categoryIndex++;
        elementBase += kCategoryElementStride;
    }
    CProductsCategory::UpdateSelectionFlags(this);
    float position[2] = {0.0f, 0.0f};
    int showSelection = 0;
    if (selectedCategoryIndex != -1)
        showSelection = 1;
    Layout_SetElementEnabled((*(void* volatile*)&layout), 0, showSelection);
    if (selectedCategoryIndex != -1) {
        Layout_GetElementPosition((*(void* volatile*)&layout), 0, position);
        int idx = selectedCategoryIndex;
        int selectedY = selectionY[idx];
        float selectedPositionY = (float)(double)selectedY;
        position[1] = selectedPositionY;
        Layout_SetElementPosition((*(void* volatile*)&layout), 0, position);
    }
    return 1;
}

// Handle cancel or a confirm/cancel button activation and begin pressed feedback.
int CPurchaseDialog::ProcessInput() {
    int captionPosition[2];
    // Raw vector header: data address, element count, capacity; IDs are in data.
    int activatedElementVectorWords[3];
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     (void*)s_inputManagerDtorRecord_PurchaseDialog);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)s_gameInputManager;
    }
    int cancelPressed = Input_IsActionPressed(GetInputManagerSlot(), kActionCancelPurchase, -1);
    if (cancelPressed != 0) {
        result = kPurchaseCancelled;
        Sound_PlayByName(g_soundManager, purchaseDialogStrings.cancelSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog,
                         kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
        return 1;
    }
    register int zero = 0;
    activatedElementVectorWords[0] = zero;
    activatedElementVectorWords[1] = zero;
    activatedElementVectorWords[2] = zero;
    ActivatedUnwindCleanup activatedCleanup(&activatedElementVectorWords);
    if (Layout_CollectActivated(layout, activatedElementVectorWords, 0) == 0) {
        if (activatedElementVectorWords[0] != 0) {
            activatedElementVectorWords[1] = activatedElementVectorWords[1] - *(int*)&activatedElementVectorWords[1];
            activatedCleanup.release();
            appFreeUni__FPv((void*)activatedElementVectorWords[0]);
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
            activatedCleanup.release();
            appFreeUni__FPv((void*)activatedElementVectorWords[0]);
        }
        return 0;
    }
    selectedElement = selectedButton;
    Layout_SetElementValueState(layout, selectedButton, kPressedElementState);
    Layout_SetElementPlayerFlag(layout, selectedElement, 0, kPlayerInteractionFlag);
    captionPosition[0] = 0;
    captionPosition[1] = 0;
    Layout_GetElementPosition(layout, selectedElement + kButtonCaptionBase, (float*)captionPosition);
    *(float*)&captionPosition[0] += kFloat_1_PurchaseDialog;
    *(float*)&captionPosition[1] += kFloat_1_PurchaseDialog;
    Layout_SetElementPosition(layout, selectedElement + kButtonCaptionBase, (float*)captionPosition);
    selectionDelayFrames = kSelectionDelayFrames;
    Sound_PlayByName(g_soundManager, purchaseDialogStrings.clickSound, 0, 0, 0, 0, kFloat_0_5_PurchaseDialog,
                     kFloat_200_PurchaseDialogSound, kFloat_1_PurchaseDialog);
    if (selectedButton == kConfirmButton) {
        result = kPurchaseConfirmed;
    } else {
        result = kPurchaseCancelled;
    }
    if (activatedElementVectorWords[0] != 0) {
        activatedElementVectorWords[1] = activatedElementVectorWords[1] - *(int*)&activatedElementVectorWords[1];
        activatedCleanup.release();
        appFreeUni__FPv((void*)activatedElementVectorWords[0]);
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
