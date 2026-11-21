#include <decomp/static_init.h>
#include <decomp/record_sizes.h>
#include "classes/CClubHouseMsg.h"
#include "classes/CMsgDraw.h"
#include "iff/IffStruct.h"
#include "game/chatmsg_match.h"
#include "classes/UITextTable.h"
#define AsCClubHouseMsgFields(p) ((CClubHouseMsgFields*)(p))

#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u802D9CC4 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    LocalNString_u802D9CC4() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    LocalNString_u802D9CC4(const char* text) { MWString_ConstructCString(this, text); }
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
    ~LocalNString_u802D9CC4() { if (flags >> 31) appFreeUni__FPv(data); }
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

// Translation unit 0x802D9CC4-0x802DABF4 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "game/singletons.h"
#pragma auto_inline off
#include "sdk_protos.h"
#include "classes/CItemInfo.h"

#define AsCItemInfo(p) ((CItemInfo*)(p))


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

// Produced by CPangya2Shop_GetSelectedProductInfo, rather than an on-disk IFF record.
struct ItemInfoDisplayData {
    int price;
    short power;
    short control;
    short spin;
    char reserved[4];
    char nameMessage[sizeof(((IffItemCommon*)0)->name)];
    char descriptionMessage[sizeof(((IffItemCommon*)0)->name)];
};
typedef char ItemInfoStringPool_size[(sizeof(ItemInfoStringPool) == 0xAC) ? 1 : -1];
typedef char ItemInfoDisplayData_name[((int)&((ItemInfoDisplayData*)0)->nameMessage == 0x0E) ? 1 : -1];
typedef char ItemInfoDisplayData_description[((int)&((ItemInfoDisplayData*)0)->descriptionMessage == 0x36) ? 1 : -1];
enum ItemInfoTextId { kPriceText = 0x1E, kPowerText, kControlText, kSpinText };
// These are logical InputManager action IDs, not Wii Remote button masks.
enum ItemInfoInputAction {
    kDismissActionPrimary = 0x17, kDismissActionSecondary = 4,
    kItemDetailsAction = 0x14, kAnyInputDevice = -1
};
enum { kDetailsLayoutKind = 5, kDetailsElementCount = 5,
       kStatusNumberCount = 4, kNoScriptEntry = -1,
       kScriptEntryIdByteOffset = 0x0C };

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

void CMsgDraw_UpdateReveal(void*);
void Text_DrawGlyphs(void*);
int CMsgDraw_ResetTextState(void*);
int CMsgDraw_SetDrawEnabled(void*, int);
void Layout_Update(void*);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetEnabled(void*, int);
int CClubHouseMsg_UpdateReveal(void*);
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
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void* CBpCtrl_PollAllEntriesLoaded(void*);
void* UITextTable_SetInteger(void*, int, int);
void* BpObject_StartScriptByName(void*, const char*, int);
void* CFontNumDraw_RegisterEntries(void*, void*, int, void*, void*);
void* UITextTable_SetEntryResourceName(void*, int, const char*);
int CClubHouseMsg_SetTextFromGroup(void*, void*, void*, int);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*);
void PointerPairList_RemoveByKey(void*, void*);
void CSharedFileHandle_Destroy(void*, int);
void UIGroup_GetPosition(void*, void*);
void Text_SetPosition(void*, void*);
int UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void UIGroup_GetSourceSize(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
void WString_ReplaceRange(void*, int, int, void*, void*, void*);
int MessageTable_FindTextByName(void*, void*);
void Text_ExpandTaggedText(void*, void*, int, int);
void Text_SetContent(void*, void*);
int InputManager_Init(void*);
void InputManager_Dtor(void*);
int Input_IsActionPressed(void*, int, int);
void __dl__FPv(void*);
int CClubHouseMsg_ResetTextState(void*);
int CClubHouseMsg_DrawText(void*);
void Layout_Destroy(void*, int);
void* __nw__FUl(unsigned int);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void CBpCtrl_CreateEntry(void*, const char*, void*, int);
void* IdMap_Lookup(void*, int);
void* CFontNumDraw_CreateInVector(void*);
void* __ct__13CClubHouseMsgFv(void*);
void CBpCtrl_DeleteEntryById(void*, int);
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
            __ct__7CBpCtrlFv(s_bpCtrl);
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

class MessageResourceInterface { public: 
virtual void Destroy(int deleteStorage);
};
// (The font-number registry singleton Sing_805DC830 is set up inline the same way; a helper changed the code.)
// Release the item-detail layout, script registration, message and numeric text table.
void* CItemInfo_dtor(void* self, int shouldFree) {
    if (self != 0) {
        *(void**)self = __vt__9CItemInfo;
        void* p = AsCItemInfo(self)->layout;
        if (p != 0) {
            Layout_Destroy(p, 1);
            AsCItemInfo(self)->layout = 0;
        }
        if (AsCItemInfo(self)->bpEntryId != kNoScriptEntry) {
            EnsureBpCtrl();
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCItemInfo(self)->bpEntryId);
            AsCItemInfo(self)->bpEntryId = kNoScriptEntry;
        }
        void* p18 = AsCItemInfo(self)->clubHouseMessage;
        if (p18 != 0) {
            if (p18 != 0) {
                ((MessageResourceInterface*)p18)->Destroy(1);
            }
            AsCItemInfo(self)->clubHouseMessage = 0;
        }
        UITextTable_ClearEntries(AsCItemInfo(self)->textTable);
        if (AsCItemInfo(self)->textTable != 0) {
            if (Sing_805DC830() == 0) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumDrawRegistryDtorRegistration_ItemInfo);
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), AsCItemInfo(self)->textTable);
        }
        if (shouldFree > 0) __dl__FPv(self);
    }
    return self;
}

// Create and register the layout, script, message and numeric table for item details.
void* CItemInfo_Ctor(void* self) {
    *(void**)self = __vt__9CItemInfo;
    struct ItemContainer {
        char storage[0x5c];
        ItemContainer(int kind) { Layout_Init(this, kind); }
    };
    void* layout = new ItemContainer(kDetailsLayoutKind);
    AsCItemInfo(self)->layout = layout;
    AsCItemInfo(self)->bpObject = 0;
    AsCItemInfo(self)->bpEntryId = kNoScriptEntry;
    AsCItemInfo(self)->itemTexture = 0;
    (*(int*)&AsCItemInfo(self)->textTable) = 0;
    struct ItemMessage {
        char storage[0x20];
        ItemMessage() { __ct__13CClubHouseMsgFv(this); }
    };
    void* message = new ItemMessage();
    AsCItemInfo(self)->clubHouseMessage = message;
    Layout_LoadExtended(AsCItemInfo(self)->layout, 0, itemInfoStrings.layout);
    EnsureBpCtrl();
    CBpCtrl_CreateEntry(Sing_805DC5F0(), itemInfoStrings.scriptResource, (void*)((volatile char*)self + kScriptEntryIdByteOffset), 0);
    EnsureBpCtrl();
    void* scriptRegistry = *(void* volatile*)&Sing_805DC5F0();
    int scriptEntryId = (*(volatile int*)&AsCItemInfo(self)->bpEntryId);
    void* scriptObject = IdMap_Lookup((void*)scriptRegistry, scriptEntryId);
    int textRegistryAddress = (int)Sing_805DC830();
    (*(void**)&AsCItemInfo(self)->bpObject) = scriptObject;
    if (textRegistryAddress == 0) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumDrawRegistryDtorRegistration_ItemInfo);
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    AsCItemInfo(self)->textTable = CFontNumDraw_CreateInVector(Sing_805DC830());
    return self;
}

// Draw the item-detail layout, optional preview texture, numeric values and message.
int CItemInfo_Draw(void* self) {
    BpObject_TryInvokeCurrentCallback((*(void**)&AsCItemInfo(self)->bpObject), 0);
    BpObject_Draw((*(void**)&AsCItemInfo(self)->bpObject));
    Layout_DrawWithOpacity(AsCItemInfo(self)->layout, kFloat_1_ItemInfo);
    if (((RenderHost*)(*(void**)&AsCItemInfo(self)->itemTexture))->IsLoaded()) {
        int rect[4] = {0, 0, 0, 0};
        Layout_ReadElementRect(AsCItemInfo(self)->layout, 0, rect);
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
        Engine_SetFlexibleVertexFormat(0x11F005, bottomRightX, bottomLeftY, bottomLeftX, topRightY, topRightX, topLeftY, topLeftX);
        ((RenderHost*)(*(void**)&AsCItemInfo(self)->itemTexture))->Bind();
        DrawPrimitiveUP(0x98, GX_GetPrimitiveCount(4, 0x98), verts.v, 0x18);
    }
    UITextTable_Draw(AsCItemInfo(self)->textTable);
    CClubHouseMsg_DrawText(AsCItemInfo(self)->clubHouseMessage);
    return 1;
}

// Accept a non-null preview texture for the item-detail pane.
int CItemInfo_SetItemTexture(void* self, void* value) {
    if (value == 0) {
        return 0;
    }
    (*(void**)&AsCItemInfo(self)->itemTexture) = value;
    return 1;
}

// Unregister and destroy the message group, text renderer and shared resource.
void* CClubHouseMsg_dtor(void* self, int shouldFree) {
    if (self != 0) {
        void* g = Sing_805DC650();
        *(void**)self = __vt__13CClubHouseMsg;
        if (g == 0) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_messageDrawPairListDtorRegistration_ItemInfo);
                s_pointerPairListInitialized = 1;
            }
            g = s_messageDrawPairList;
            Sing_805DC650() = g;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), AsCClubHouseMsgFields(self)->textObject);
        if (AsCClubHouseMsgFields(self)->group != 0) {
            if (AsCClubHouseMsgFields(self)->group != 0) {
                ((MessageResourceInterface*)AsCClubHouseMsgFields(self)->group)->Destroy(1);
            }
            AsCClubHouseMsgFields(self)->group = 0;
        }
        if (AsCClubHouseMsgFields(self)->textObject != 0) {
            if (AsCClubHouseMsgFields(self)->textObject != 0) {
                ((MessageResourceInterface*)AsCClubHouseMsgFields(self)->textObject)->Destroy(1);
            }
            AsCClubHouseMsgFields(self)->textObject = 0;
        }
        if (AsCClubHouseMsgFields(self)->sharedFile != 0) {
            CSharedFileHandle_Destroy(AsCClubHouseMsgFields(self)->sharedFile, 1);
            AsCClubHouseMsgFields(self)->sharedFile = 0;
        }
        if ((unsigned int)self + 0x14 != 0) {
            if ((AsCClubHouseMsgFields(self)->textFlags >> 31) != 0) {
                appFreeUni__FPv(AsCClubHouseMsgFields(self)->textData);
            }
        }
        if (shouldFree > 0) __dl__FPv(self);
    }
    return self;
}

// Hide item details when any of the three shared dismiss actions is pressed.
int CItemInfo_HandleDismissInput(void* self) {
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init((void*)s_gameInputManager);
            __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_ItemInfo);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = (void*)s_gameInputManager;
    }
    // Dismiss through the shared action mapper; no hardware-specific poll occurs here.
    void* manager = Sing_805DC5C8();
    if (!Input_IsActionPressed(manager, kDismissActionPrimary, kAnyInputDevice) && !Input_IsActionPressed(manager, kDismissActionSecondary, kAnyInputDevice) && !Input_IsActionPressed(manager, kItemDetailsAction, kAnyInputDevice)) return 0;
    CClubHouseMsg_ResetTextState(AsCItemInfo(self)->clubHouseMessage);
    BpObject_SetEnabled((*(void**)&AsCItemInfo(self)->bpObject), 0);
    AsUITextTable(AsCItemInfo(self)->textTable)->drawEnabled = 0; // original text table +0x28
    Sound_PlayByName(g_soundManager, itemInfoStrings.cancelSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo, kFloat_1_ItemInfo);
    Sound_PlayByName(g_soundManager, itemInfoStrings.closeSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo, kFloat_1_ItemInfo);
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
int CClubHouseMsg_SetTextFromGroup(void* messageObject, void* messageKey, void* sourceGroup, int resizeToGroup) {
    CClubHouseMsgFields* self = AsCClubHouseMsgFields(messageObject);
    MessageTextScratch scratch;
    int tint;
    int opacity;
    int existingTextLength;
    char* resetText;
    size_t resetTextLength;
    int messageIndex;
    void* expandedText;
    UIGroup_GetPosition(sourceGroup, scratch.positionStorage);
    Text_SetPosition(self->textObject, scratch.positionStorage);
    if (sourceGroup != 0) {
        tint = UIGroup_GetTintColor(sourceGroup);
        opacity = UIGroup_GetCombinedOpacity(sourceGroup);
        int nonzeroOpacityMask = (-opacity | opacity) >> 31;
        int doubledOpacityMinusOne = (opacity << 1) - 1;
        *(unsigned int*)&((CMsgDraw*)self->textObject)->colorMultiplier = // 0x802DA018: packed tint at +0x68
            (unsigned int)(nonzeroOpacityMask & doubledOpacityMinusOne) |
            (((unsigned int)tint << 24) | (((unsigned int)tint << 8) & 0xff0000u) | (((unsigned int)tint >> 8) & 0xff00u));
    }
    CMsgDraw_ClearFormattingAndRevealAll(self->textObject);
    if (resizeToGroup != 0) {
        UIGroup_GetSourceSize(sourceGroup, scratch.out + 4);
        CMsgDraw_SetLayoutSize(self->textObject, scratch.out + 4);
        resetText = s_clubHouseTextResetString;
        existingTextLength = ((self->textFlags >> 31) == 0) ? (*(unsigned char*)&self->textFlags & 0x7f) : (int)self->textLength;
        scratch.item = *(volatile unsigned char*)scratch.out;
        resetTextLength = wcslen((const wchar_t*)resetText);
        WString_ReplaceRange(&self->textFlags, 0, existingTextLength, resetText, resetText + resetTextLength * 2, &scratch.item);
    }
    messageIndex = MessageTable_FindTextByName(self->group, messageKey);
    Text_ExpandTaggedText(self->group, &self->textFlags, messageIndex, 0);
    expandedText = ((self->textFlags >> 31) == 0) ? (void*)((wchar_t*)&self->textFlags + 1) : self->textData;
    Text_SetContent(self->textObject, expandedText);
    scratch.tailItem = *(volatile unsigned char*)&scratch.a8;
    WString_ReplaceRange(&self->textFlags, 0, -1, 0, 0, &scratch.tailItem);
    return 1;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Advance the text renderer reveal animation.
int CClubHouseMsg_UpdateReveal(void* self) {
    CMsgDraw_UpdateReveal(AsCClubHouseMsgFields(self)->textObject);
    return 1;
}

// Draw the message glyphs.
int CClubHouseMsg_DrawText(void* self) {
    Text_DrawGlyphs(AsCClubHouseMsgFields(self)->textObject);
    return 1;
}

// Reset the message reveal state.
int CClubHouseMsg_ResetTextState(void* self) {
    CMsgDraw_ResetTextState(AsCClubHouseMsgFields(self)->textObject);
    return 1;
}

// Enable or disable the message text renderer.
int CClubHouseMsg_SetDrawEnabled(void* self, int value) {
    CMsgDraw_SetDrawEnabled(AsCClubHouseMsgFields(self)->textObject, value);
    return 1;
}

// Advance the item-detail layout animation.
int CItemInfo_UpdateLayout(void* self) {
    Layout_Update(AsCItemInfo(self)->layout);
    return 1;
}

#pragma inline_depth(1)
// Advance both layout animation and message reveal.
int CItemInfo_UpdateLayoutAndMessage(void* self) {
    Layout_Update(AsCItemInfo(self)->layout);
    CClubHouseMsg_UpdateReveal(AsCItemInfo(self)->clubHouseMessage);
    return 1;
}
#pragma inline_depth(255)

// Report whether the shared message resource has a loaded buffer.
int CClubHouseMsg_IsResourceLoaded(void* self) {
    void* p = AsCClubHouseMsgFields(self)->sharedFile;
    if (p == 0 || CSharedFilePtr_GetLoadedBuffer(p) == 0) {
        return 0;
    }
    return 1;
}

// Create the message group and registered text renderer from the loaded resource.
int CClubHouseMsg_CreateTextFromResource(void* self) {
    void* p = AsCClubHouseMsgFields(self)->sharedFile;
    int value = p ? (int)(long)CSharedFilePtr_GetLoadedBuffer(p) : 0;
    WFixedFont_CreateMsgEcbResource(AsCClubHouseMsgFields(self)->layoutManager, &AsCClubHouseMsgFields(self)->group, (void*)value, 1);
    WFixedFont_CreateAndRegisterMsgDraw(AsCClubHouseMsgFields(self)->layoutManager, &AsCClubHouseMsgFields(self)->textObject, 1);
    return 1;
}

#pragma inline_depth(1)
// Wait for the item-info layout, shared scripts and message resource to finish loading.
int CItemInfo_IsLoaded(void* self) {
    if (Layout_PollLoadComplete(AsCItemInfo(self)->layout) == 0) return 0;
    // Same lazy BP-controller set-up as EnsureBpCtrl(), kept inline: a helper call changes the code under inline_depth(1).
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemInfo);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) == 0) return 0;
    return CClubHouseMsg_IsResourceLoaded(AsCItemInfo(self)->clubHouseMessage);
}

// Populate price, Power, Control, Spin, item name and description from the shop selection.
int CItemInfo_SetDisplayedData(void* self, ItemInfoDisplayData* data) {
    UITextTable_SetInteger(AsCItemInfo(self)->textTable, kPriceText, data->price);
    UITextTable_SetInteger(AsCItemInfo(self)->textTable, kPowerText, data->power);
    UITextTable_SetInteger(AsCItemInfo(self)->textTable, kControlText, data->control);
    UITextTable_SetInteger(AsCItemInfo(self)->textTable, kSpinText, data->spin);
    void* messageGroup = BpObject_GetOrCreateNamedGroup((*(void**)&AsCItemInfo(self)->bpObject), itemInfoStrings.itemNameGroup);
    CClubHouseMsg_SetTextFromGroup(AsCItemInfo(self)->clubHouseMessage, data->nameMessage, messageGroup, 0);
    messageGroup = BpObject_GetOrCreateNamedGroup((*(void**)&AsCItemInfo(self)->bpObject), itemInfoStrings.detailsGroup);
    CClubHouseMsg_SetTextFromGroup(AsCItemInfo(self)->clubHouseMessage, data->descriptionMessage, messageGroup, 0);
    return 1;
}

// Prepare the detail script and numeric fonts, keeping the pane hidden.
int CItemInfo_InitializeDetailsUI(void* self) {
    Layout_SetModeByte(AsCItemInfo(self)->layout, 0);
    BpObject_StartScriptByName((*(void**)&AsCItemInfo(self)->bpObject), itemInfoStrings.scriptName, 1);
    BpObject_SetEnabled((*(void**)&AsCItemInfo(self)->bpObject), 0);
    for (int i = 0; i < kDetailsElementCount; ++i) Layout_SetElementEnabled(AsCItemInfo(self)->layout, i, 0);
    CFontNumDraw_RegisterEntries(AsCItemInfo(self)->textTable, s_itemInfoNumericEntryDefinitions, kStatusNumberCount, (*(void**)&AsCItemInfo(self)->bpObject), itemInfoStrings.statusNumberFont);
    UITextTable_SetEntryResourceName(AsCItemInfo(self)->textTable, kPriceText, itemInfoStrings.priceNumberFont);
    AsUITextTable(AsCItemInfo(self)->textTable)->drawEnabled = 0;
    CClubHouseMsg_CreateTextFromResource(AsCItemInfo(self)->clubHouseMessage);
    return 1;
}
#pragma inline_depth(255)

#pragma inline_depth(1)
// Reset and show the detail pane, then play its opening sound.
int CItemInfo_Show(void* self) {
    CClubHouseMsg_ResetTextState(AsCItemInfo(self)->clubHouseMessage);
    BpObject_SetEnabled((*(void**)&AsCItemInfo(self)->bpObject), 1);
    AsUITextTable(AsCItemInfo(self)->textTable)->drawEnabled = 1;
    Sound_PlayByName(g_soundManager, itemInfoStrings.openSound, 0, 0, 0, 0, kFloat_0_5_ItemInfo, kFloat_200_ItemInfo, kFloat_1_ItemInfo);
    return 1;
}
#pragma inline_depth(255)

typedef LocalNString_u802D9CC4 LocalD9D18;

}
struct ClubHousePrefix {
    char bytes[0x14];
    ClubHousePrefix() {
        *(void**)this = __vt__13CClubHouseMsg;
        *(unsigned int*)(bytes + 8) = 0;
        *(unsigned int*)(bytes + 0xc) = 0;
        *(unsigned int*)(bytes + 0x10) = 0;
    }
};
struct CClubHouseMsg {
    ClubHousePrefix prefix;
    WString text;
    CClubHouseMsg();
};
// Acquire the localized message file and the shared chat font resource.
CClubHouseMsg::CClubHouseMsg() {
    void* self = this;
    LocalD9D18 local;
    NString_AssignCString(&local, clubHouseMessageResourceStrings);
    void* path;
    if ((local.flags >> 31) == 0) {
        path = (void*)((char*)&local + 1);
    } else {
        path = (void*)local.data;
    }
    AsCClubHouseMsgFields(self)->sharedFile = SharedFile_Create(path, 0);
    AsCClubHouseMsgFields(self)->layoutManager = ((CChatMsg*)g_chatMsg)->defaultFixedFont;

}
extern "C" {

// Toggle the overlay that marks the selected item as unowned.
int CItemInfo_SetNotBuyGroupEnabled(void* self, void* value) {
    void* overlay = BpObject_GetOrCreateNamedGroup((*(void**)&AsCItemInfo(self)->bpObject), itemInfoStrings.notOwnedGroup);
    UIGroup_SetEnabled(overlay, (int)value);
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
