// Item-detail pane (CItemInfo) and the localized club-house message box (CClubHouseMsg).
// The pane shows an item's price, stats, name and description; the message box
// places and reveals localized text inside it.
#include <decomp/static_init.h>
#include <decomp/record_sizes.h>
#include "classes/CClubHouseMsg.h"
#include "classes/CMsgDraw.h"
#include "iff/IffStruct.h"
#include "game/chatmsg_match.h"
#include "classes/UITextTable.h"
#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u802D9CC4 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    LocalNString_u802D9CC4() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    LocalNString_u802D9CC4(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    LocalNString_u802D9CC4(const char* text) {
        LocalNString_u802D9CC4* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    ~LocalNString_u802D9CC4() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "game/singletons.h"
#pragma auto_inline off
#include "sdk_protos.h"
#include "classes/CItemInfo.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"

// Packed resource-name pool; member sizes preserve the original string offsets.
struct ItemInfoStringPool {
    char layout[16];
    char scriptResource[16];
    char scriptName[12];
    char statusNumberFont[18];
    char priceNumberFont[16];
    char cancelSound[23];
    char closeSound[16];
    char itemNameGroup[16];
    char detailsGroup[14];
    char openSound[15];
    char notOwnedGroup[10];
};

typedef char ItemInfoStringPool_size[(sizeof(ItemInfoStringPool) == 0xAC) ? 1 : -1];
typedef char ItemInfoDisplayData_name[((int)&((ItemInfoDisplayData*)0)->nameMessage == 0x0E) ? 1 : -1];
typedef char ItemInfoDisplayData_description[((int)&((ItemInfoDisplayData*)0)->descriptionMessage == 0x36) ? 1 : -1];
enum ItemInfoTextId {
    kPriceText = 0x1E,
    kPowerText,
    kControlText,
    kSpinText
};
// These are logical InputManager action IDs, not Wii Remote button masks.
enum ItemInfoInputAction {
    kDismissActionPrimary = 0x17,
    kDismissActionSecondary = 4,
    kItemDetailsAction = 0x14,
    kAnyInputDevice = -1
};
enum {
    kDetailsLayoutKind = 5,
    kDetailsElementCount = 5,
    kStatusNumberCount = 4,
    kNoScriptEntry = -1,
    kScriptEntryIdByteOffset = 0x0C
};

extern "C" {

struct VertBlock {
    float v[24];
};

struct RenderHost {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void Bind();
    virtual int IsLoaded();
};

extern float kFloat_6_2831855;
extern float s_itemInfoUnitAngleConstant0;
extern float s_itemInfoUnitAngleConstant1;
extern float s_itemInfoUnitAngleConstant2;
extern float s_itemInfoUnitAngleConstant3;
extern float s_itemInfoUnitAngleConstant4;
extern float s_itemInfoUnitAngleConstant5;
extern float s_itemInfoUnitAngleConstant6;
extern float s_itemInfoUnitAngleConstant7;
extern float s_itemInfoUnitAngleConstant8;
extern ItemInfoStringPool itemInfoStrings;
extern char s_gameInputManager[];

void Text_DrawGlyphs(void*);
void Layout_Update(void*);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetEnabled(void*, int);
void WFixedFont_CreateMsgEcbResource(void*, void*, void*, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void BpObject_SetEnabled(void*, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void NString_AssignCString(void*, const char*);
void* SharedFile_Create(void*, int);
void appFreeUni__FPv(void*);

extern const float kFloat_0_5_ItemInfo;
extern const float kFloat_200_ItemInfo;
extern const float kFloat_1_ItemInfo;
extern void* g_chatMsg;
extern void* g_soundManager;
extern char clubHouseMessageResourceStrings[];
extern char __vt__13CClubHouseMsg[];
extern char s_clubHouseTextResetString[];
extern char s_messageDrawPairList[];
extern ItemInfoStringPool itemInfoStrings;
extern char s_messageDrawPairListDtorRegistration_ItemInfo[];
extern char s_inputManagerDtorRegistration_ItemInfo[];
extern signed char s_pointerPairListInitialized;
extern signed char s_inputManagerInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern char s_fontNumDrawRegistryDtorRegistration_ItemInfo[];
extern char __vt__9CItemInfo[];
extern VertBlock s_itemPreviewQuadTemplate;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ItemInfo[];
extern char s_itemInfoNumericEntryDefinitions[];
void* Layout_PollLoadComplete(void*);
void Layout_SetModeByte(void*, int);
void Layout_SetElementEnabled(void*, int, int);
void CBpCtrl_dtor(void);
void* UITextTable_SetInteger(void*, int, int);
void* BpObject_StartScriptByName(void*, const char*, int);
void* UITextTable_SetEntryResourceName(void*, int, const char*);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*);
void PointerPairList_RemoveByKey(void*, void*);
void UIGroup_GetPosition(void*, void*);
void Text_SetPosition(void*, void*);
int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void UIGroup_GetSourceSize(void*, void*);
void WString_ReplaceRange(void*, int, int, void*, void*, void*);
int MessageTable_FindTextByName(void*, void*);
void Text_ExpandTaggedText(void*, void*, int, int);
void Text_SetContent(void*, void*);
int InputManager_Init(void*);
void InputManager_Dtor(void*);
int Input_IsActionPressed(void*, int, int);
void __dl__FPv(void*);
void Layout_Destroy(void*, int);
void* __nw__FUl(unsigned int);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void* IdMap_Lookup(void*, int);
void UITextTable_ClearEntries(void*);
void PointerArray_DeleteValue(void*, void*);
void PointerVector_InitReserved(void*);
void PointerVector_DestroyStorage(void);
void UITextTable_Draw(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_ReadElementRect(void*, int, void*);
void Engine_SetFlexibleVertexFormat(int, float, float, float, float, float, float, float);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);

// Lazy singleton set-up: construct the global BP controller (s_bpCtrl) on first use and publish it in Sing_805DC5F0.
static inline void EnsureBpCtrl() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemInfo);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}
// Initialize full-turn cache 0 for the item-details module.
void ItemInfo_InitFullTurnCache0() {
    s_itemInfoUnitAngleConstant0 = kFloat_6_2831855;
}

// Initialize full-turn cache 1 for the item-details module.
void ItemInfo_InitFullTurnCache1() {
    s_itemInfoUnitAngleConstant1 = kFloat_6_2831855;
}

// Initialize full-turn cache 2 for the item-details module.
void ItemInfo_InitFullTurnCache2() {
    s_itemInfoUnitAngleConstant2 = kFloat_6_2831855;
}

// Initialize full-turn cache 3 for the item-details module.
void ItemInfo_InitFullTurnCache3() {
    s_itemInfoUnitAngleConstant3 = kFloat_6_2831855;
}

// Initialize full-turn cache 4 for the item-details module.
void ItemInfo_InitFullTurnCache4() {
    s_itemInfoUnitAngleConstant4 = kFloat_6_2831855;
}

// Initialize full-turn cache 5 for the item-details module.
void ItemInfo_InitFullTurnCache5() {
    s_itemInfoUnitAngleConstant5 = kFloat_6_2831855;
}

// Initialize full-turn cache 6 for the item-details module.
void ItemInfo_InitFullTurnCache6() {
    s_itemInfoUnitAngleConstant6 = kFloat_6_2831855;
}

// Initialize full-turn cache 7 for the item-details module.
void ItemInfo_InitFullTurnCache7() {
    s_itemInfoUnitAngleConstant7 = kFloat_6_2831855;
}

// Initialize full-turn cache 8 for the item-details module.
void ItemInfo_InitFullTurnCache8() {
    s_itemInfoUnitAngleConstant8 = kFloat_6_2831855;
}

class MessageResourceInterface {
public:
    virtual void Destroy(int deleteStorage);
};
// (The font-number registry singleton Sing_805DC830 is set up inline the same way; a helper changed the code.)
// Release the item-detail layout, script registration, message and numeric text table.
CItemInfo::~CItemInfo() {
    *(void**)this = __vt__9CItemInfo;
    void* p = layout;
    if (p != 0) {
        Layout_Destroy(p, 1);
        layout = 0;
    }
    if (bpEntryId != kNoScriptEntry) {
        EnsureBpCtrl();
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(bpEntryId);
        bpEntryId = kNoScriptEntry;
    }
    void* p18 = clubHouseMessage;
    if (p18 != 0) {
        if (p18 != 0) {
            ((MessageResourceInterface*)p18)->Destroy(1);
        }
        clubHouseMessage = 0;
    }
    UITextTable_ClearEntries(textTable);
    if (textTable != 0) {
        if (Sing_805DC830() == 0) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                         s_fontNumDrawRegistryDtorRegistration_ItemInfo);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), textTable);
    }
}

// Create and register the layout, script, message and numeric table for item details.
CItemInfo::CItemInfo() {
    *(void**)this = __vt__9CItemInfo;
    struct ItemContainer {
        char storage[0x5c];
        ItemContainer(int kind) {
            Layout_Init(this, kind);
        }
    };
    void* layout = new ItemContainer(kDetailsLayoutKind);
    this->layout = layout;
    bpObject = 0;
    bpEntryId = kNoScriptEntry;
    itemTexture = 0;
    (*(int*)&textTable) = 0;
    struct ItemMessage {
        char storage[0x20];
        ItemMessage() {
            ((CClubHouseMsg*)this)->__ct();
        }
    };
    void* message = new ItemMessage();
    clubHouseMessage = message;
    Layout_LoadExtended(this->layout, 0, itemInfoStrings.layout);
    EnsureBpCtrl();
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(itemInfoStrings.scriptResource, (void*)((volatile char*)this + kScriptEntryIdByteOffset), 0);
    EnsureBpCtrl();
    void* scriptRegistry = *(void* volatile*)&Sing_805DC5F0();
    int scriptEntryId = (*(volatile int*)&bpEntryId);
    void* scriptObject = IdMap_Lookup((void*)scriptRegistry, scriptEntryId);
    int textRegistryAddress = (int)Sing_805DC830();
    (*(void**)&bpObject) = scriptObject;
    if (textRegistryAddress == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     s_fontNumDrawRegistryDtorRegistration_ItemInfo);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    textTable = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
}

// Draw the item-detail layout, optional preview texture, numeric values and message.
int CItemInfo::Draw() {
    BpObject_TryInvokeCurrentCallback((*(void**)&bpObject), 0);
    BpObject_Draw((*(void**)&bpObject));
    Layout_DrawWithOpacity(layout, kFloat_1_ItemInfo);
    if (((RenderHost*)(*(void**)&itemTexture))->IsLoaded()) {
        int rect[4] = {0, 0, 0, 0};
        Layout_ReadElementRect(layout, 0, rect);
        VertBlock verts = s_itemPreviewQuadTemplate;
        float topLeftX = (float)rect[0];
        float topLeftY = (float)rect[1];
        float topRightX = (float)rect[2];
        float topRightY = (float)rect[1];
        float bottomLeftX = (float)rect[0];
        float bottomLeftY = (float)rect[3];
        float bottomRightX = (float)rect[2];
        float bottomRightY = (float)rect[3];
        verts.v[0] = topLeftX;
        verts.v[1] = topLeftY;
        verts.v[6] = topRightX;
        verts.v[7] = topRightY;
        verts.v[12] = bottomLeftX;
        verts.v[13] = bottomLeftY;
        verts.v[18] = bottomRightX;
        verts.v[19] = bottomRightY;
        Engine_SetFlexibleVertexFormat(0x11F005, bottomRightX, bottomLeftY, bottomLeftX, topRightY, topRightX, topLeftY,
                                       topLeftX);
        ((RenderHost*)(*(void**)&itemTexture))->Bind();
        DrawPrimitiveUP(0x98, GX_GetPrimitiveCount(4, 0x98), verts.v, 0x18);
    }
    UITextTable_Draw(textTable);
    ((CClubHouseMsg*)clubHouseMessage)->DrawText();
    return 1;
}

// Accept a non-null preview texture for the item-detail pane.
int CItemInfo::SetItemTexture(int value) {
    if (value == 0) {
        return 0;
    }
    itemTexture = value;
    return 1;
}

// Unregister and destroy the message group, text renderer and shared resource.
CClubHouseMsg::~CClubHouseMsg() {
    vtbl = __vt__13CClubHouseMsg;
    void* g = Sing_805DC650();
    if (g == 0) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                     s_messageDrawPairListDtorRegistration_ItemInfo);
            s_pointerPairListInitialized = 1;
        }
        g = s_messageDrawPairList;
        Sing_805DC650() = g;
    }
    PointerPairList_RemoveByKey(Sing_805DC650(), textObject);
    if (group != 0) {
        if (group != 0) {
            ((MessageResourceInterface*)group)->Destroy(1);
        }
        group = 0;
    }
    if (textObject != 0) {
        if (textObject != 0) {
            ((MessageResourceInterface*)textObject)->Destroy(1);
        }
        textObject = 0;
    }
    if (sharedFile != 0) {
        CSharedFileHandle::Destroy(sharedFile, 1);
        sharedFile = 0;
    }
    // The original compiles this always-true address test on the text field; it must stay to match.
    if ((unsigned int)this + 0x14 != 0) {
        if ((textFlags >> 31) != 0) {
            appFreeUni__FPv(textData);
        }
    }
}

// Hide item details when any of the three shared dismiss actions is pressed.
int CItemInfo::HandleDismissInput() {
    if (GetInputManagerSlot() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init((void*)s_gameInputManager);
            __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_ItemInfo);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = (void*)s_gameInputManager;
    }
    // Dismiss through the shared action mapper; no hardware-specific poll occurs here.
    void* manager = GetInputManagerSlot();
    if (!Input_IsActionPressed(manager, kDismissActionPrimary, kAnyInputDevice) &&
        !Input_IsActionPressed(manager, kDismissActionSecondary, kAnyInputDevice) &&
        !Input_IsActionPressed(manager, kItemDetailsAction, kAnyInputDevice))
        return 0;
    ((CClubHouseMsg*)clubHouseMessage)->ResetTextState();
    BpObject_SetEnabled((*(void**)&bpObject), 0);
    AsUITextTable(textTable)->drawEnabled = 0; // original text table +0x28
    Sound_PlayByName(g_soundManager, itemInfoStrings.cancelSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo,
                     kFloat_1_ItemInfo);
    Sound_PlayByName(g_soundManager, itemInfoStrings.closeSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo,
                     kFloat_1_ItemInfo);
    return 1;
}

// Stack allocator tags are intentionally read through volatile bytes to preserve the ABI.
struct MessageTextScratch {
    unsigned char a8;
    unsigned char pad9[3];
    unsigned char tailItem;
    unsigned char padd[3];
    unsigned char item;
    unsigned char padh[3];
    unsigned char out[8];
    unsigned char padi[4];
    unsigned char positionStorage[4];
};

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Place and tint a localized message using its UI group, optionally adopting its text box size.
int CClubHouseMsg::SetTextFromGroup(void* messageKey, void* sourceGroup, int resizeToGroup) {
    MessageTextScratch scratch;
    int tint;
    int opacity;
    int existingTextLength;
    char* resetText;
    size_t resetTextLength;
    int messageIndex;
    void* expandedText;
    UIGroup_GetPosition(sourceGroup, scratch.positionStorage);
    Text_SetPosition(textObject, scratch.positionStorage);
    if (sourceGroup != 0) {
        tint = UIGroup_GetTintColor(sourceGroup);
        opacity = UIGroup_GetCombinedOpacity(sourceGroup);
        int nonzeroOpacityMask = (-opacity | opacity) >> 31;
        int doubledOpacityMinusOne = (opacity << 1) - 1;
        *(unsigned int*)&((CMsgDraw*)textObject)->colorMultiplier = // packed tint at +0x68
            (unsigned int)(nonzeroOpacityMask & doubledOpacityMinusOne) |
            (((unsigned int)tint << 24) | (((unsigned int)tint << 8) & 0xff0000u) |
             (((unsigned int)tint >> 8) & 0xff00u));
    }
    ((CMsgDraw*)textObject)->ClearFormattingAndRevealAll();
    if (resizeToGroup != 0) {
        UIGroup_GetSourceSize(sourceGroup, scratch.out + 4);
        ((CMsgDraw*)textObject)->SetLayoutSize((float*)(scratch.out + 4));
        resetText = s_clubHouseTextResetString;
        existingTextLength = ((textFlags >> 31) == 0) ? (*(unsigned char*)&textFlags & 0x7f) : (int)textLength;
        scratch.item = *(volatile unsigned char*)scratch.out;
        resetTextLength = wcslen((const wchar_t*)resetText);
        WString_ReplaceRange(&textFlags, 0, existingTextLength, resetText, resetText + resetTextLength * 2,
                             &scratch.item);
    }
    messageIndex = MessageTable_FindTextByName(group, messageKey);
    Text_ExpandTaggedText(group, &textFlags, messageIndex, 0);
    expandedText = ((textFlags >> 31) == 0) ? (void*)((wchar_t*)&textFlags + 1) : textData;
    Text_SetContent(textObject, expandedText);
    scratch.tailItem = *(volatile unsigned char*)&scratch.a8;
    WString_ReplaceRange(&textFlags, 0, -1, 0, 0, &scratch.tailItem);
    return 1;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Advance the text renderer reveal animation.
int CClubHouseMsg::UpdateReveal() {
    ((CMsgDraw*)textObject)->UpdateReveal();
    return 1;
}

// Draw the message glyphs.
int CClubHouseMsg::DrawText() {
    Text_DrawGlyphs(textObject);
    return 1;
}

// Reset the message reveal state.
int CClubHouseMsg::ResetTextState() {
    ((CMsgDraw*)textObject)->ResetTextState();
    return 1;
}

// Enable or disable the message text renderer.
int CClubHouseMsg::SetDrawEnabled(int value) {
    ((CMsgDraw*)textObject)->SetDrawEnabled(value);
    return 1;
}

// Advance the item-detail layout animation.
int CItemInfo::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

#pragma inline_depth(1)
// Advance both layout animation and message reveal.
int CItemInfo::UpdateLayoutAndMessage() {
    Layout_Update(layout);
    ((CClubHouseMsg*)clubHouseMessage)->UpdateReveal();
    return 1;
}
#pragma inline_depth(255)

// Report whether the shared message resource has a loaded buffer.
int CClubHouseMsg::IsResourceLoaded() {
    void* p = sharedFile;
    if (p == 0 || CSharedFilePtr_GetLoadedBuffer(p) == 0) {
        return 0;
    }
    return 1;
}

// Create the message group and registered text renderer from the loaded resource.
int CClubHouseMsg::CreateTextFromResource() {
    void* p = sharedFile;
    int value = p ? (int)(long)CSharedFilePtr_GetLoadedBuffer(p) : 0;
    WFixedFont_CreateMsgEcbResource(layoutManager, &group, (void*)value, 1);
    WFixedFont_CreateAndRegisterMsgDraw(layoutManager, &textObject, 1);
    return 1;
}

#pragma inline_depth(1)
// Wait for the item-info layout, shared scripts and message resource to finish loading.
int CItemInfo::IsLoaded() {
    if (Layout_PollLoadComplete(layout) == 0)
        return 0;
    // Same lazy BP-controller set-up as EnsureBpCtrl(), kept inline: a helper call changes the code under inline_depth(1).
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemInfo);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0)
        return 0;
    return ((CClubHouseMsg*)clubHouseMessage)->IsResourceLoaded();
}

// Populate price, Power, Control, Spin, item name and description from the shop selection.
int CItemInfo::SetDisplayedData(ItemInfoDisplayData* data) {
    UITextTable_SetInteger(textTable, kPriceText, data->price);
    UITextTable_SetInteger(textTable, kPowerText, data->power);
    UITextTable_SetInteger(textTable, kControlText, data->control);
    UITextTable_SetInteger(textTable, kSpinText, data->spin);
    void* messageGroup = BpObject_GetOrCreateNamedGroup((void*)bpObject, itemInfoStrings.itemNameGroup);
    ((CClubHouseMsg*)clubHouseMessage)->SetTextFromGroup(data->nameMessage, messageGroup, 0);
    messageGroup = BpObject_GetOrCreateNamedGroup((void*)bpObject, itemInfoStrings.detailsGroup);
    ((CClubHouseMsg*)clubHouseMessage)->SetTextFromGroup(data->descriptionMessage, messageGroup, 0);
    return 1;
}

// Prepare the detail script and numeric fonts, keeping the pane hidden.
int CItemInfo::InitializeDetailsUI() {
    Layout_SetModeByte(layout, 0);
    BpObject_StartScriptByName((*(void**)&bpObject), itemInfoStrings.scriptName, 1);
    BpObject_SetEnabled((*(void**)&bpObject), 0);
    for (int i = 0; i < kDetailsElementCount; ++i)
        Layout_SetElementEnabled(layout, i, 0);
    ((CFontNumDraw*)textTable)
        ->RegisterEntries(s_itemInfoNumericEntryDefinitions, kStatusNumberCount, (*(void**)&bpObject),
                          itemInfoStrings.statusNumberFont);
    UITextTable_SetEntryResourceName(textTable, kPriceText, itemInfoStrings.priceNumberFont);
    AsUITextTable(textTable)->drawEnabled = 0;
    ((CClubHouseMsg*)clubHouseMessage)->CreateTextFromResource();
    return 1;
}
#pragma inline_depth(255)

#pragma inline_depth(1)
// Reset and show the detail pane, then play its opening sound.
int CItemInfo::Show() {
    ((CClubHouseMsg*)clubHouseMessage)->ResetTextState();
    BpObject_SetEnabled((*(void**)&bpObject), 1);
    AsUITextTable(textTable)->drawEnabled = 1;
    Sound_PlayByName(g_soundManager, itemInfoStrings.openSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo,
                     kFloat_1_ItemInfo);
    return 1;
}
#pragma inline_depth(255)

typedef LocalNString_u802D9CC4 LocalD9D18;
}
// Acquire the localized message file and the shared chat font resource.
CClubHouseMsg::CClubHouseMsg() {
    vtbl = __vt__13CClubHouseMsg;
    group = 0;
    layoutManager = 0;
    textObject = 0;
    textFlags = 0;
    textLength = 0;
    textData = 0;
    LocalD9D18 local;
    NString_AssignCString(&local, clubHouseMessageResourceStrings);
    void* path;
    if ((local.flags >> 31) == 0) {
        path = (void*)((char*)&local + 1);
    } else {
        path = (void*)local.data;
    }
    sharedFile = SharedFile_Create(path, 0);
    layoutManager = ((CChatMsg*)g_chatMsg)->defaultFixedFont;
}
extern "C" {

// Toggle the overlay that marks the selected item as unowned.
int CItemInfo::SetNotBuyGroupEnabled(int enabled) {
    void* overlay = BpObject_GetOrCreateNamedGroup((void*)bpObject, itemInfoStrings.notOwnedGroup);
    UIGroup_SetEnabled(overlay, enabled);
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_0, ItemInfo_InitFullTurnCache0);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_1, ItemInfo_InitFullTurnCache1);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_2, ItemInfo_InitFullTurnCache2);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_3, ItemInfo_InitFullTurnCache3);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_4, ItemInfo_InitFullTurnCache4);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_5, ItemInfo_InitFullTurnCache5);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_6, ItemInfo_InitFullTurnCache6);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_7, ItemInfo_InitFullTurnCache7);
STATIC_INIT_ENTRY(".CRT$XCU80462528", _ctors_u802D9CC4_8, ItemInfo_InitFullTurnCache8);
#pragma pop
