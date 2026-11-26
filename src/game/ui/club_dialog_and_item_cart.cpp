// The club dialog (CClubDialog) and the item cart (CItemCart), two clubhouse shop screens.
// The dialog handles button input and reports a mode-specific result. The cart shows up to
// eight equipped item slots and adds items, reporting when it is full.
//
// The AsC* helpers borrow shared shop objects without taking ownership of them.

#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "iff/IffStruct.h"
// Borrow the shared-document layout of an opaque singleton pointer.
static inline CSharedDoc* AsCSharedDoc(void* object) {
    return (CSharedDoc*)object;
}

#include "game/singletons.h"
#include "classes/CItemCart.h"
#include "classes/CClubDialog.h"
#include "classes/CShopSubMenuB.h"
#include "classes/CShopMenu.h"
#include "classes/CBpCtrl.h"

// Club dialog: element 9 supplies message bounds; buttons 10/11 confirm/cancel,
// 12/13 are the two alternatives, and 14 acknowledges. Decorations are button + 5.
// Modes 0/1/4/6 show 10/11, mode 2 shows 12/13, mode 5 shows 14. The result
// mapping is in ClubDialogConstants; business labels are not established for every mode.
// Item cart: elements 0..7 receive input, 8 supplies message position, 9..16 supply
// icon rectangles. All positions/rectangles are in layout screen coordinates.

// Borrow the shop-menu layout without changing ownership.
static inline CShopMenu* AsCShopMenu(void* object) {
    return (CShopMenu*)object;
}

// Borrow the submenu layout without changing ownership.
static inline CShopSubMenuB* AsCShopSubMenuB(void* object) {
    return (CShopSubMenuB*)object;
}

// Borrow the club-dialog layout without changing ownership.
static inline CClubDialog* AsCClubDialog(void* object) {
    return (CClubDialog*)object;
}

// Borrow the item-cart layout without changing ownership.
static inline CItemCart* AsCItemCart(void* object) {
    return (CItemCart*)object;
}

enum ClubDialogConstants {
    // Names describe the observable result codes; the callers' business meanings remain unproven.
    kModeConfirmResult2 = 0,
    kModeConfirmResult3 = 1,
    kModeChooseResults4Or5 = 2,
    kModeConfirmResult6 = 4,
    kModeAcknowledge = 5,
    kModeAlternateConfirmResult3 = 6,
    kInputActionCancel = 0x17, // Logical action ID, not a Wii Remote button mask.
    kAnyInputPlayer = -1,
    kNoDialogResult = 0,
    kDialogResultCancel = 1,
    kDialogResultMode0Accept = 2,
    kDialogResultMode1Or6Accept = 3,
    kDialogResultMode2First = 4,
    kDialogResultMode2Second = 5,
    kDialogResultMode4Accept = 6,
    kFirstDialogButton = 10,
    kLastDialogButton = 14,
    kConfirmButton = 10,
    kFirstChoiceButton = 12,
    kAcknowledgeButton = 14,
    kDialogButtonDecorationOffset = 5,
    kDialogMessageElement = 9,
    kSelectionFeedbackFrames = 3,
    kInactiveDelay = -1,
    kItemSlotCount = 8,
    kEmptyInventorySlot = -1,
    kFirstItemIconElement = 9,
    kItemMessageElement = 8,
    kAllLayoutPlayers = 5,
    kElementStateIdle = 0,
    kElementStatePressed = 2,
    kTexturedQuadVertexFormat = 0x11F005,
    kTexturedVertexStride = 0x18,
    kIconExpansionDivisor = 10,
    kGxTriangleStrip = 0x98
};

// Partial views for save fields not yet exposed by SavePlayerRecord.h.
struct SavedItemSlots {
    char prefix[0x31];
    signed char itemInventoryIndices[kItemSlotCount];
};
struct SavedItemQuantities {
    char prefix[0x11C];
    short quantities[1];
}; // Trailing inventory-indexed array.

// The supplied cart records expose the common item header and a 0x2C0 stride; their remaining format is opaque.
struct CartItemRecord {
    IffItemCommon common;
    char remainingData[0x2C0 - sizeof(IffItemCommon)];
};
typedef char CartItemRecord_size[(sizeof(CartItemRecord) == 0x2C0) ? 1 : -1];

typedef char SavedItemSlots_offset[((int)&((SavedItemSlots*)0)->itemInventoryIndices == 0x31) ? 1 : -1];
typedef char SavedItemQuantities_offset[((int)&((SavedItemQuantities*)0)->quantities == 0x11C) ? 1 : -1];

// Packed string pool: member sizes include the terminating NUL.
struct ClubDialogStringPool {
    char layoutFile[0xD];
    char closeSound[0x10];
    char openSound[0xF];
    char cancelSound[0x17];
    char selectSound[1]; // Only the address of this final entry is needed here.
};
struct ItemCartStringPool {
    char layoutFile[14];
    char scriptFile[14];
    char scriptName[10];
    char errorSound[9];
    char itemClickSound[14];
    char cancelSound[23];
};

extern "C" {

extern void Layout_Update(void*);
extern int Layout_PollLoadComplete(void*);
extern void Layout_Destroy(void*, int);
extern void Layout_DrawWithOpacity(void*, float);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern int MenuMessage_DrawText(void*);
extern void MenuMessage_UpdateReveal(void*);
extern void MenuMessage_Dtor(void*, int);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_WriteElementRect(void*, int, void*);
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementPosition(void*, int, void*);
extern void Layout_GetElementPosition(void*, int, void*);
extern void Layout_SetElementPlayerFlag(void*, int, int, int);
extern void Layout_SetElementValueState(void*, int,
                                        int); // Select element value state (0 = idle, 2 = pressed feedback).
extern int Input_IsActionPressed(void*, int, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern int Layout_CollectActivated(void*, void*, int);
extern void appFreeUni__FPv(void*);
extern void Layout_SetModeByte(void*, int); // Set layout byte +0x3C; initialized to zero here, broader role unresolved.
extern void MenuMessage_SetPosition(void*, void*);
extern void MenuMessage_SetLayoutSize(void*, void*);
extern void* memset(void*, int, unsigned long);
extern void __dl__FPv(void*);
extern const float kFloat_1_805E09B8;
extern const float kFloat_0_5_ClubDialog; // 0.5f: dialog sound parameter.
extern const float kFloat_200_ClubDialog; // 200.0f: leading float of the shared constant storage.
extern const float kFloat_1_ItemCart;
extern char __vt__11CClubDialog[];
extern char clubDialogStrings[];
extern void* g_soundManager;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_gameInputManagerDestructorRecord[];
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDestructorRecord[];
extern void* __nw__FUl(unsigned long);
extern int jumptable_804B6F74[];
extern void Layout_Init(void*, int);
extern void MenuMessage_Ctor(void*);
extern void Layout_LoadExtended(void*, int, char*);
extern void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern void MenuMessage_ResetTextState(void*);
extern void MenuMessage_SetText(void*, int);
extern int IdMap_Lookup(void*, int);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void CBpCtrl_dtor(void*);
extern void* __register_global_object(void*, void*, void*);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void* UIGroup_GetOrCreateById(void*, const char*);
extern void UIGroup_SetEnabled(void*, int);
extern void Layout_SetElementEnabled(void*, int, int);
extern void MenuMessage_SetPosition(void*, void*);
extern int ItemInventory_FindItemIndex(void*, void*);
extern signed char s_bpCtrlInitialized;
extern void* g_soundManager;
extern char itemCartStrings[];
extern char __vt__9CItemCart[];
extern int clubDialogMessageIds[];           // Message ID indexed by dialog mode.
extern const char* itemCartIconGroupNames[]; // Eight script icon group names.
extern const float kItemCartQuadTemplate[];
extern const float kFloat_0_5_ItemCart; // 0.5f: item-cart sound parameter.
extern const float kFloat_200_ItemCart; // 200.0f: item-cart sound parameter.
extern const float kFloat_6_2831855;    // One turn in radians (6.2831855f).
extern float s_itemCartFullTurnConstant;
extern CSharedDoc* g_sharedDoc;
extern void Engine_SetFlexibleVertexFormat(int);
extern void* GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, void*, void*, int);
extern int Layout_HitTestPlayer(void*, int);

struct ItemQuadVertices {
    float f[24];
};
struct DrawElem {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void draw();
    virtual int test();
    char data[0x24];
};
struct ClubDialogLayoutStorage {
    char data[0x5c];
    // Initialize the dialog's owned layout storage.
    ClubDialogLayoutStorage(int x) {
        Layout_Init(this, x);
    }
};
struct ClubDialogMessageStorage {
    char data[0xc];
    // Initialize the dialog's owned message storage.
    ClubDialogMessageStorage() {
        MenuMessage_Ctor(this);
    }
};
struct ItemCartLayoutStorage {
    char data[0x5c];
    // Initialize the cart's owned layout storage.
    ItemCartLayoutStorage(int x) {
        Layout_Init(this, x);
    }
};
struct ItemCartMessageStorage {
    char data[0xc];
    // Initialize the cart's owned message storage.
    ItemCartMessageStorage() {
        MenuMessage_Ctor(this);
    }
};

struct ActivatedList {
    int* items;
    unsigned int count;
    int capacity;
    // Release the collected layout element IDs; retain the original unsigned count-clear expression.
    ~ActivatedList() {
        if (items != 0) {
            count -= ((char*)this + count) - (char*)this;
            appFreeUni__FPv(items);
        }
    }
};

// Handle logical cancel or activated layout buttons, latch the mode-specific result, and start press feedback.
int CClubDialog::ProcessInput() {
    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_gameInputManagerDestructorRecord);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    // Replace or bypass this logical-action query to customize cancellation independently of controller drivers.
    if (Input_IsActionPressed(GetInputManagerSlot(), kInputActionCancel, kAnyInputPlayer)) {
        result = kDialogResultCancel;
        Sound_PlayByName(g_soundManager, ((ClubDialogStringPool*)clubDialogStrings)->cancelSound, 0, 0, 0, 0,
                         kFloat_0_5_ClubDialog, kFloat_200_ClubDialog, kFloat_1_805E09B8);
        return 1;
    }
    void* self = this;
    int activatedByteOffset = 0; // Preserve the original four-byte induction variable.
    ActivatedList activated = {0, 0, 0};
    // Confirmation follows activated layout elements, rather than a second direct button query.
    if (!Layout_CollectActivated(AsCClubDialog(self)->layout, &activated, 0)) {
        return 0;
    }
    int selected = -1;
    for (unsigned int i = 0; i < activated.count; ++i, activatedByteOffset += 4) {
        int elementId = *(int*)((unsigned char*)activated.items + activatedByteOffset);
        if (elementId >= kFirstDialogButton && elementId <= kLastDialogButton) {
            selected = elementId;
            break;
        }
    }
    switch (AsCClubDialog(self)->mode) {
    case kModeConfirmResult2:
        if (selected == kConfirmButton)
            AsCClubDialog(self)->result = kDialogResultMode0Accept;
        else
            AsCClubDialog(self)->result = kDialogResultCancel;
        break;
    case kModeConfirmResult3:
    case kModeAlternateConfirmResult3:
        if (selected == kConfirmButton)
            AsCClubDialog(self)->result = kDialogResultMode1Or6Accept;
        else
            AsCClubDialog(self)->result = kDialogResultCancel;
        break;
    case kModeChooseResults4Or5:
        if (selected == kFirstChoiceButton)
            AsCClubDialog(self)->result = kDialogResultMode2First;
        else
            AsCClubDialog(self)->result = kDialogResultMode2Second;
        break;
    case kModeConfirmResult6:
        if (selected == kConfirmButton)
            AsCClubDialog(self)->result = kDialogResultMode4Accept;
        else
            AsCClubDialog(self)->result = kDialogResultCancel;
        break;
    case kModeAcknowledge:
        if (selected == kAcknowledgeButton)
            AsCClubDialog(self)->result = kDialogResultCancel;
        break;
    }
    AsCClubDialog(self)->selectedElement = selected;
    Layout_SetElementValueState(AsCClubDialog(self)->layout, selected, kElementStatePressed);
    Layout_SetElementPlayerFlag(AsCClubDialog(self)->layout, AsCClubDialog(self)->selectedElement, 0,
                                kAllLayoutPlayers);
    float decorationPosition[2] = {0.0f, 0.0f};
    Layout_GetElementPosition(AsCClubDialog(self)->layout,
                              AsCClubDialog(self)->selectedElement + kDialogButtonDecorationOffset, decorationPosition);
    decorationPosition[0] += kFloat_1_805E09B8;
    decorationPosition[1] += kFloat_1_805E09B8;
    Layout_SetElementPosition(AsCClubDialog(self)->layout,
                              AsCClubDialog(self)->selectedElement + kDialogButtonDecorationOffset, decorationPosition);
    AsCClubDialog(self)->selectionDelayFrames = kSelectionFeedbackFrames;
    Sound_PlayByName(g_soundManager, ((ClubDialogStringPool*)clubDialogStrings)->selectSound, 0, 0, 0, 0,
                     kFloat_0_5_ClubDialog, kFloat_200_ClubDialog, kFloat_1_805E09B8);
    return 1;
}

#pragma opt_common_subs off
// Allocate layout/message objects, load the item script, and clear the pending eight-slot selection.
// playerRecord is borrowed writable save data; itemCount is a 32-bit count passed via void*.
// Icon/record arrays are supplied separately and are not deleted by the cart destructor.
// Kept as a named member, not a constructor: the caller initialises it inside its own storage object.
CItemCart* CItemCart::Init(void* playerRecord, void* itemCount) {
    vtbl = __vt__9CItemCart;
    void* p = new ItemCartLayoutStorage(5);
    layout = p;
    itemIcons = 0;
    itemRecords = 0;
    playerData = playerRecord;
    this->itemCount = itemCount;
    refreshDelayFrames = kInactiveDelay;
    scriptObject = 0;
    scriptEntryId = -1;
    p = new ItemCartMessageStorage();
    message = p;
    selectedItemIndex = -1;
    Layout_LoadExtended(layout, 0, ((ItemCartStringPool*)itemCartStrings)->layoutFile);
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(((ItemCartStringPool*)itemCartStrings)->scriptFile, &scriptEntryId, 0);
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    scriptObject = IdMap_Lookup(Sing_805DC5F0(), scriptEntryId);
    memset(pendingInventoryIndices, kEmptyInventorySlot, kItemSlotCount);
    return this;
}
#pragma opt_common_subs reset

// Enable the mode-specific button pair or acknowledgement button, reset the message, and play the open sound.
// mode must index clubDialogMessageIds; unsupported values still index that table.
int CClubDialog::SetMode(int mode) {
    result = kNoDialogResult;
    this->mode = mode;
    switch (mode) {
    case kModeConfirmResult2:
    case kModeConfirmResult3:
    case kModeConfirmResult6:
    case kModeAlternateConfirmResult3:
        for (int i = kFirstDialogButton; i <= kLastDialogButton; ++i) {
            if ((unsigned)(i - kFirstDialogButton) <= 1) {
                Layout_SetElementPlayerFlagsAndState(layout, i, 1);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(layout, i, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 0);
            }
        }
        break;
    case kModeChooseResults4Or5:
        for (int i = kFirstDialogButton; i <= kLastDialogButton; ++i) {
            if ((unsigned)(i - kFirstChoiceButton) <= 1) {
                Layout_SetElementPlayerFlagsAndState(layout, i, 1);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(layout, i, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 0);
            }
        }
        break;
    case kModeAcknowledge:
        for (int i = kFirstDialogButton; i <= kLastDialogButton; ++i) {
            if (i == kAcknowledgeButton) {
                Layout_SetElementPlayerFlagsAndState(layout, i, 1);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 1);
            } else {
                Layout_SetElementPlayerFlagsAndState(layout, i, 0);
                Layout_SetElementPlayerFlagsAndState(layout, i + kDialogButtonDecorationOffset, 0);
            }
        }
        break;
    }
    MenuMessage_ResetTextState(message);
    MenuMessage_SetText(message, clubDialogMessageIds[mode]);
    Layout_Update(layout);
    Sound_PlayByName(g_soundManager, ((ClubDialogStringPool*)clubDialogStrings)->openSound, 0, 0, 0, 0,
                     kFloat_0_5_ClubDialog, kFloat_200_ClubDialog, kFloat_1_805E09B8);
    return 1;
}

// Advance the club dialog layout once.
int CClubDialog::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Advance the item cart layout once.
int CItemCart::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Report whether the shop menu layout has completed loading.
int CShopMenu::IsLayoutLoaded() {
    return Layout_PollLoadComplete(layout) != 0;
}

// Draw the dialog at full opacity and its revealed message.
int CClubDialog::Draw() {
    float t = kFloat_1_805E09B8;
    Layout_DrawWithOpacity(layout, t);
    MenuMessage_DrawText(message);
    return 1;
}

// Advance layout/text and accept input only when press feedback is inactive.
int CClubDialog::Update() {
    Layout_Update(layout);
    MenuMessage_UpdateReveal(message);
    if (CClubDialog::UpdateSelectionDelay(this) == 0) {
        ProcessInput();
    }
    return 1;
}

// Destroy owned layout and message objects, then optionally release the dialog allocation.
CClubDialog::~CClubDialog() {
    void* q = layout;
    vtable = (void*)__vt__11CClubDialog;
    if (q != 0) {
        Layout_Destroy(q, 1);
        layout = 0;
    }
    void* r = message;
    if (r != 0) {
        MenuMessage_Dtor(r, 1);
        message = 0;
    }
}

// Run the script draw callback, then draw the layout, item icons, and message.
int CItemCart::Draw() {
    BpObject_TryInvokeCurrentCallback((*(void**)&scriptObject), 0);
    BpObject_Draw((*(void**)&scriptObject));
    Layout_DrawWithOpacity(layout, kFloat_1_ItemCart);
    CItemCart::DrawItems(this);
    MenuMessage_DrawText(message);
    return 1;
}

// Publish a latched result after press feedback ends; play the cancel sound for closing results.
// Return 0 without writing resultOut while pending; return 1 after writing an int result.
// Reading does not consume the result; repeated successful calls repeat text/sound effects.
int CClubDialog::TryGetResult(void* resultOut) {
    int result = this->result;
    if (result == kNoDialogResult || selectionDelayFrames != kInactiveDelay) {
        return 0;
    }
    *(int*)resultOut = result;
    MenuMessage_ResetTextState(message);
    int r0 = this->result;
    if (r0 == kDialogResultCancel || r0 == kDialogResultMode4Accept) {
        Sound_PlayByName(g_soundManager, ((ClubDialogStringPool*)clubDialogStrings)->closeSound, 0, 0, 0, 0,
                         kFloat_0_5_ClubDialog, kFloat_200_ClubDialog, kFloat_1_805E09B8);
    }
    return 1;
}

// Require both layout and shared script-controller resources to finish loading.
int CShopSubMenuB::IsLoaded() {
    void* q = layout;
    if (Layout_PollLoadComplete(q) == 0) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRecord);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = (void*)s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0) {
        return 0;
    }
    return 1;
}

// Set up a dialog in caller-owned storage with no active mode/result, and load its layout.
// Kept as a named member, not a constructor: the caller initialises it inside its own storage object.
CClubDialog* CClubDialog::Init() {
    vtable = (void*)__vt__11CClubDialog;
    ClubDialogLayoutStorage* q = new ClubDialogLayoutStorage(5);
    void* r31 = q;
    layout = r31;
    mode = -1;
    result = kNoDialogResult;
    ClubDialogMessageStorage* r = new ClubDialogMessageStorage();
    void* r31b = r;
    message = r31b;
    selectionDelayFrames = kInactiveDelay;
    selectedElement = -1;
    Layout_LoadExtended(layout, 0, ((ClubDialogStringPool*)clubDialogStrings)->layoutFile);
    return this;
}

// Count down press feedback, restore the selected decoration position, and reset button states.
int CClubDialog::UpdateSelectionDelay(void* object) {
    void* self = object;
    int remainingFrames = AsCClubDialog(self)->selectionDelayFrames;
    if (remainingFrames == kInactiveDelay)
        return 0;
    AsCClubDialog(self)->selectionDelayFrames = remainingFrames - 1;
    if (remainingFrames > 0)
        return 1;
    float pressedOffset = kFloat_1_805E09B8;
    int buttonElement = kFirstDialogButton;
    int decorationPositionBits[2];
    for (; buttonElement <= kLastDialogButton; buttonElement++) {
        if (buttonElement == AsCClubDialog(self)->selectedElement) {
            decorationPositionBits[0] = 0;
            decorationPositionBits[1] = 0;
            int decorationElement = buttonElement + kDialogButtonDecorationOffset;
            Layout_GetElementPosition(AsCClubDialog(self)->layout, decorationElement, decorationPositionBits);
            decorationElement = buttonElement + kDialogButtonDecorationOffset;
            *(float*)&decorationPositionBits[0] = *(float*)&decorationPositionBits[0] - pressedOffset;
            *(float*)&decorationPositionBits[1] = *(float*)&decorationPositionBits[1] - pressedOffset;
            Layout_SetElementPosition(AsCClubDialog(self)->layout, decorationElement, decorationPositionBits);
        }
        Layout_SetElementValueState(AsCClubDialog(self)->layout, buttonElement, kElementStateIdle);
    }
    AsCClubDialog(self)->selectionDelayFrames = kInactiveDelay;
    return 1;
}

// Align button decorations and configure the message position and width from layout element nine.
int CClubDialog::InitLayout(void* object) {
    void* self = object;
    Layout_SetModeByte(AsCClubDialog(self)->layout, 0);
    int messageRect[4];
    int buttonRect[4] = {0, 0, 0, 0};
    int buttonElement = kFirstDialogButton;
    for (; buttonElement <= kLastDialogButton; buttonElement++) {
        Layout_ReadElementRect(AsCClubDialog(self)->layout, buttonElement, buttonRect);
        Layout_WriteElementRect(AsCClubDialog(self)->layout, buttonElement + kDialogButtonDecorationOffset, buttonRect);
    }
    Layout_SetElementEnabled(AsCClubDialog(self)->layout, kDialogMessageElement, 0);
    float messagePosition[2];
    Layout_GetElementPosition(AsCClubDialog(self)->layout, kDialogMessageElement, messagePosition);
    MenuMessage_SetPosition(AsCClubDialog(self)->message, messagePosition);
    Layout_ReadElementRect(AsCClubDialog(self)->layout, kDialogMessageElement, messageRect);
    messagePosition[0] = (float)(messageRect[2] - messageRect[0]);
    MenuMessage_SetLayoutSize(AsCClubDialog(self)->message, messagePosition);
    return 1;
}

// Find the icon record for an equipped slot, using pending inventory indices during selection.
// slotIndex is 0..7. Save/pending bytes hold inventory indices, not item type IDs.
// Return an index into itemRecords/itemIcons, or -1 if no record matches.
// The shared member pendingItemIds is an inventory-index array despite its name.
static inline int FindIconIndexForSlot(char* self, int slotIndex) {
    // Keep byte cursors: indexing the arrays directly changes induction registers in the caller.
    int recordByteOffset, iconIndex;
    iconIndex = 0;
    recordByteOffset = 0;
    for (; iconIndex < (*(int*)&AsCItemCart(self)->itemCount);
         recordByteOffset += sizeof(CartItemRecord), ++iconIndex) {
        if (AsCItemCart(self)->selectedItemIndex == -1) {
            int inventoryIndex = ItemInventory_FindItemIndex(
                AsCSharedDoc(g_sharedDoc)->itemInventory,
                (void*)((CartItemRecord*)((char*)(*(void**)&AsCItemCart(self)->itemRecords) + recordByteOffset))
                    ->common.typeId);
            signed char equippedInventoryIndex =
                ((SavedItemSlots*)(slotIndex + (int)AsCItemCart(self)->playerData))->itemInventoryIndices[0];
            if (equippedInventoryIndex == inventoryIndex)
                return iconIndex;
        }
        if (AsCItemCart(self)->selectedItemIndex != -1) {
            int inventoryIndex = ItemInventory_FindItemIndex(
                AsCSharedDoc(g_sharedDoc)->itemInventory,
                (void*)((CartItemRecord*)((char*)(*(void**)&AsCItemCart(self)->itemRecords) + recordByteOffset))
                    ->common.typeId);
            signed char equippedInventoryIndex = ((CItemCart*)(self + slotIndex))->pendingInventoryIndices[0];
            if (equippedInventoryIndex == inventoryIndex)
                return iconIndex;
        }
    }
    return -1;
}

// Draw up to eight equipped icons, expanding the selected or hovered icon rectangle.
int CItemCart::DrawItems(void* object) {
    char* self = (char*)object;
    // Packed occupied slots form a prefix; the first unresolved icon ends the entire draw.
    int slotIndex = 0;
    do {
        int iconIndex = FindIconIndexForSlot(self, slotIndex);
        if (iconIndex == -1)
            break;
        {
            int iconByteOffset = iconIndex * sizeof(DrawElem);
            if (((DrawElem*)((char*)(*(void**)&AsCItemCart(self)->itemIcons) + iconByteOffset))->test()) {
                int iconRect[4] = {0, 0, 0, 0};
                Layout_ReadElementRect(AsCItemCart(self)->layout, slotIndex + kFirstItemIconElement, iconRect);
                int selectedSlot = AsCItemCart(self)->selectedItemIndex;
                if (selectedSlot == slotIndex) {
                    iconRect[0] = iconRect[0] - ((iconRect[2] - iconRect[0]) / kIconExpansionDivisor - 1);
                    iconRect[2] += (iconRect[2] - iconRect[0]) / kIconExpansionDivisor + 1;
                    iconRect[1] = iconRect[1] - ((iconRect[3] - iconRect[1]) / kIconExpansionDivisor - 1);
                    iconRect[3] += (iconRect[3] - iconRect[1]) / kIconExpansionDivisor + 1;
                } else if (Layout_HitTestPlayer(AsCItemCart(self)->layout, slotIndex)) {
                    iconRect[0] = iconRect[0] - (iconRect[2] - iconRect[0]) / kIconExpansionDivisor;
                    iconRect[2] = iconRect[2] + (iconRect[2] - iconRect[0]) / kIconExpansionDivisor;
                    iconRect[1] = iconRect[1] - (iconRect[3] - iconRect[1]) / kIconExpansionDivisor;
                    iconRect[3] = iconRect[3] + (iconRect[3] - iconRect[1]) / kIconExpansionDivisor;
                }
                // Build four screen-space corners. Expansion deliberately uses the already
                // modified left/top edge when computing right/bottom; it is not symmetric.
                ItemQuadVertices vertices = *(ItemQuadVertices*)kItemCartQuadTemplate;
                vertices.f[0] = (float)iconRect[0];
                vertices.f[1] = (float)iconRect[1];
                vertices.f[6] = (float)iconRect[2];
                vertices.f[7] = (float)iconRect[1];
                vertices.f[12] = (float)iconRect[0];
                vertices.f[13] = (float)iconRect[3];
                vertices.f[18] = (float)iconRect[2];
                vertices.f[19] = (float)iconRect[3];
                Engine_SetFlexibleVertexFormat(kTexturedQuadVertexFormat);
                ((DrawElem*)((char*)(*(void**)&AsCItemCart(self)->itemIcons) + iconByteOffset))->draw();
                DrawPrimitiveUP(kGxTriangleStrip, GX_GetPrimitiveCount(4, kGxTriangleStrip), vertices.f,
                                kTexturedVertexStride);
            }
        }
        ++slotIndex;
    } while (slotIndex < kItemSlotCount);
    return 1;
}

// Advance layout/text and refresh slot interaction flags after the pending feedback delay.
int CItemCart::Update(void* object) {
    void* self = object;
    Layout_Update(AsCItemCart(self)->layout);
    MenuMessage_UpdateReveal(AsCItemCart(self)->message);
    int remainingFrames = AsCItemCart(self)->refreshDelayFrames;
    if (remainingFrames != -1) {
        AsCItemCart(self)->refreshDelayFrames = remainingFrames - 1;
        if (remainingFrames <= 0) {
            int i;
            for (i = 0; i <= kItemSlotCount - 1; i++) {
                Layout_SetElementValueState(AsCItemCart(self)->layout, i, 0);
            }
            for (i = 0; i < kItemSlotCount; i++) {
                void* base = AsCItemCart(self)->playerData;
                signed char inventoryIndex = ((SavedItemSlots*)base)->itemInventoryIndices[i];
                if (inventoryIndex == -1) {
                    Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 0, kAllLayoutPlayers);
                } else {
                    Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 1, kAllLayoutPlayers);
                }
            }
            AsCItemCart(self)->refreshDelayFrames = kInactiveDelay;
            int selectedSlot = AsCItemCart(self)->selectedItemIndex;
            if (selectedSlot != -1) {
                memset(AsCItemCart(self)->pendingInventoryIndices, kEmptyInventorySlot, kItemSlotCount);
                AsCItemCart(self)->selectedItemIndex = -1;
            }
        }
    }
    return 1;
}

// Destroy the owned layout, script-controller entry, and message, then optionally free the cart.
CItemCart::~CItemCart() {
    void* q = layout;
    *(void**)this = (void*)__vt__9CItemCart;
    if (q != 0) {
        Layout_Destroy(q, 1);
        layout = 0;
    }
    if (scriptEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDestructorRecord);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(scriptEntryId);
        scriptEntryId = -1;
    }
    void* r = message;
    if (r != 0) {
        MenuMessage_Dtor(r, 1);
        message = 0;
    }
}

// Start the item script, enable occupied slots, position the message, and hide script icon groups.
int CItemCart::InitLayout(void* object) {
    void* self = object;
    register int i, j;
    Layout_SetModeByte(AsCItemCart(self)->layout, 0);
    BpObject_StartScriptByName((*(void**)&AsCItemCart(self)->scriptObject),
                               ((ItemCartStringPool*)itemCartStrings)->scriptName, 1);
    for (i = 0; i < kItemSlotCount; i++) {
        signed char c = ((SavedItemSlots*)AsCItemCart(self)->playerData)->itemInventoryIndices[i];
        if (c == -1) {
            Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 0, kAllLayoutPlayers);
        } else {
            Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 1, kAllLayoutPlayers);
        }
    }
    for (j = 8; j <= 16; j++)
        Layout_SetElementEnabled(AsCItemCart(self)->layout, j, 0);
    int buf[2] = {0, 0};
    Layout_GetElementPosition(AsCItemCart(self)->layout, kItemMessageElement, buf);
    MenuMessage_SetPosition(AsCItemCart(self)->message, buf);
    const char** p = itemCartIconGroupNames;
    for (unsigned int k = 0; k < kItemSlotCount; k++, p++) {
        void* node = UIGroup_GetOrCreateById((*(void**)&AsCItemCart(self)->scriptObject), *p);
        UIGroup_SetEnabled(node, 0);
    }
    return 1;
}

// Fill the first empty equipped slot, decrement its inventory quantity, and refresh interaction flags.
// itemId is an inventory type ID. Caller guarantees a valid inventory entry, positive
// quantity, and packed occupied slots. Return 0 when full, otherwise 1; no validation here.
int CItemCart::AddItem(int itemId) {
    void* self = this;
    register char* playerRecord = (*(char**)&playerData);
    if (((SavedItemSlots*)playerRecord)->itemInventoryIndices[kItemSlotCount - 1] != -1) {
        Sound_PlayByName(g_soundManager, ((ItemCartStringPool*)itemCartStrings)->errorSound, 0, 0, 0, 0,
                         kFloat_0_5_ItemCart, kFloat_200_ItemCart, kFloat_1_ItemCart);
        return 0;
    }
    int i;
    for (i = 0; i < kItemSlotCount; i++) {
        if (((SavedItemSlots*)(playerRecord + i))->itemInventoryIndices[0] == -1) {
            int inventoryIndex = ItemInventory_FindItemIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)itemId);
            ((SavedItemSlots*)AsCItemCart(self)->playerData)->itemInventoryIndices[i] = (signed char)inventoryIndex;
            short* quantity = &((SavedItemQuantities*)AsCItemCart(self)->playerData)->quantities[inventoryIndex];
            *quantity = *quantity - 1;
            break;
        }
    }
    for (i = 0; i < kItemSlotCount; i++) {
        signed char slotInventoryIndex = ((SavedItemSlots*)AsCItemCart(self)->playerData)->itemInventoryIndices[i];
        if (slotInventoryIndex == -1) {
            Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 0, kAllLayoutPlayers);
        } else {
            Layout_SetElementPlayerFlag(AsCItemCart(self)->layout, i, 1, kAllLayoutPlayers);
        }
    }
    Sound_PlayByName(g_soundManager, ((ItemCartStringPool*)itemCartStrings)->itemClickSound, 0, 0, 0, 0,
                     kFloat_0_5_ItemCart, kFloat_200_ItemCart, kFloat_1_ItemCart);
    return 1;
}

// Initialize this unit's angle constant to one turn during static initialization.
void InitItemCartFullTurnConstant(void) {
    float f0 = kFloat_6_2831855;
    s_itemCartFullTurnConstant = f0;
}

// Return whether the final slot is occupied; equipped items are packed from the first slot.
int CItemCart::IsFull() {
    void* base = playerData;
    signed char lastSlotInventoryIndex = ((SavedItemSlots*)base)->itemInventoryIndices[kItemSlotCount - 1];
    int nonEmptyBits = (-1 - lastSlotInventoryIndex) | (lastSlotInventoryIndex + 1);
    return (unsigned int)nonEmptyBits >> 31;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462554", _ctors_u802DC0E8_0, InitItemCartFullTurnConstant);
#pragma pop
