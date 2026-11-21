#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/MenuMessage802CE2BC.h"
#include "classes/InflateWindow802CE62C.h"
#include "classes/InflateOutput802CE62C.h"
// Translation unit 0x802CD608-0x802CE7B8 (27 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CTopMenuBase.h"
#include "classes/CMsgDraw.h"

#define AsCTopMenuBase(p) ((CTopMenuBase*)(p))

enum TopMenuConstants {
    kMenuItemElementStride = 4,
    kActionOpenOptions = 17,
    kActionCancel = 6,
    kAllInputPlayers = -1,
    kNoActivatedElement = -1,
    kPressedElementState = 2,
    kZlibBufferError = -5,
    kZlibOk = 0
};
struct TopMenuStringPool {
    char musicFile[9]; // menu.wav
    char emptySelectionError[30]; // vector::at index out of range
    char outOfRangeType[55]; // Metrowerks exception type chain
    char exitScript[4]; // out
};
struct ChatMessageFactoryView {
    char managerStorage[0x10];
    void* textFactory;
};
inline void* __register_global_object(void* object, void (*destructor)(), void* regmem) {
    return __register_global_object(object, (void*)destructor, regmem);
}
extern "C" {
extern "C" char __vt__12CTopMenuBase[];
extern "C" signed char s_inputManagerInitialized;
extern "C" char s_gameInputManager[64];extern "C" void InputManager_Init(void*);
extern "C" void InputManager_Dtor();
extern "C" int Input_IsActionPressed(void*, int, int);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void* MenuMessage_Ctor(void*);
extern "C" int MenuMessage_SetText(void*, int);
extern "C" int MenuMessage_ResetTextState(void*);
extern "C" float kFloat_6_2831855;
extern "C" float s_menuMessageTau;
extern "C" float s_inflateWindowTau;
extern "C" signed char s_fadeOverlayInitialized;
extern "C" char s_fadeOverlay[64];
extern "C" void FadeOverlay_Init(void*);
extern "C" void ResourceState_Dtor();
extern "C" int FadeState_GetState(void*);
extern "C" char s_fadeOverlayDtorRegistration_TopMenu[64];
extern "C" char s_inputManagerDtorRegistration_TopMenu[64];
extern "C" void BpObject_StartScriptByName(void*, void*, int);
extern "C" void* memcpy(void*, const void*, unsigned int);
// Initialize this module's stored two-pi constant from the shared scalar pool.
void MenuMessage_InitTauConstant() {
    s_menuMessageTau = kFloat_6_2831855;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void InflateWindow_InitTauConstant() {
    s_inflateWindowTau = kFloat_6_2831855;
}

// Set the rendered message's packed color, preserving the volatile text-pointer read.
int MenuMessage_SetColorMultiplier(void* self, int* packedColor) {
    void* text = *reinterpret_cast<void* volatile*>((char*)&AsMenuMessage802CE2BC(self)->text);
    int color = *packedColor;
    ((CMsgDraw*)text)->colorMultiplier = color;
    return 1;
}

extern "C" void Text_SetPosition(void*, float*);
extern "C" void CMsgDraw_UpdateReveal(void*);
extern "C" void Text_DrawGlyphs(void*);
extern "C" bool CMsgDraw_SetLayoutSize(void*, float*);
extern "C" int CMsgDraw_ResetTextState(void*);
extern "C" bool Layout_HitTestPlayer(void*, int);
extern "C" int Layout_PollLoadComplete(void*);
extern "C" int CPackTextureResource_IsLoadFinished(void*);
extern "C" void CPackTextureResource_PollLoad(void*);
extern "C" void __ct__7CBpCtrlFv(void*);
extern "C" void CBpCtrl_dtor();
extern "C" int CBpCtrl_PollAllEntriesLoaded(void*);
extern "C" char s_bpCtrl[];
extern "C" char s_bpCtrlDtorRegistration_ClubSelect[];
extern "C" signed char s_bpCtrlInitialized;
extern "C" void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern "C" int CMsgDraw_ClearFormattingAndRevealAll(void*);
extern "C" void* Text_ExpandTaggedText(void*, int*, int, int);
extern "C" void* Text_SetContent(void*, void*);
extern "C" void WString_ReplaceRange(int*, int, int, int, int, unsigned char*);
extern "C" void appFreeUni__FPv(void*);
extern "C" void PairVector_InitReserved(void*);
extern "C" void PointerPairList_RemoveByKey(void*, void*);
extern "C" void MessageDrawPairList_Dtor();
extern "C" char s_messageDrawPairList[];
extern "C" signed char s_pointerPairListInitialized;
extern "C" char s_messageDrawPairListDtorRegistration_TopMenu[];
extern "C" void Layout_Destroy(void*, int);
extern "C" void CBpCtrl_DeleteEntryById(void*, int);
extern "C" void* UIGroup_GetOrCreateById(void*, int);
extern "C" int Layout_CollectActivated(void*, void*, int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void __dla__FPv(void*);
extern "C" void __dl__FPv(void*);
extern "C" char topMenuAudioStrings[256];
extern "C" void Allocation_Dtor(void*);
extern "C" int ResourceTable_FindEntryByName(void*, void*, void*);
extern "C" void CPackTextureResource_Init(void*, void*, void*, int);
extern "C" void Layout_Init(void*, int);
extern "C" void Layout_LoadExtended(void*, int, void*);
extern "C" void CBpCtrl_CreateEntry(void*, void*, int*, int);
extern "C" void* IdMap_Lookup(void*, int);
extern "C" void SoundManager_RequestBgmChange(void*, void*, int, int);
extern "C" void* g_soundManager;
extern "C" signed char s_resourceTableInitialized;
extern "C" void* s_resourceTableStorage;
extern "C" char s_resourceTableDtorRegistration_TopMenu[];
extern "C" char __vt__Q23std11logic_error[64];
extern "C" char s_sharedCharDeleterVtable[64];
extern "C" char __vt__Q23std12out_of_range[64];
extern "C" void __dt__Q23std12out_of_rangeFv(void*);

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Create owned layout/BP/texture/message resources and request the menu music.
void* CTopMenuBase_Ctor(void* self, void* layoutName, void* unused, void* bpResourceName, void* textureName, int itemCount, void* elementIdTable) {
    struct Message {
        char data[12];
        Message() { MenuMessage_Ctor(this); }
    };
    struct Texture {
        char data[0x20];
        Texture(void* name, void* data, int value) { CPackTextureResource_Init(this, name, data, value); }
    };
    struct Layout {
        char data[0x5c];
        Layout(int kind) { Layout_Init(this, kind); }
    };
    void* archiveName = textureName;
    void* vtable = &__vt__12CTopMenuBase;
    AsCTopMenuBase(self)->vtbl = vtable;
    (*(int*)&AsCTopMenuBase(self)->layout) = 0;
    AsCTopMenuBase(self)->bpObject = 0;
    AsCTopMenuBase(self)->menuItemCount = itemCount;
    (*(int*)&AsCTopMenuBase(self)->message) = 0;
    AsCTopMenuBase(self)->optionDialog = 0;
    AsCTopMenuBase(self)->elementIds = (int)elementIdTable;
    AsCTopMenuBase(self)->borrowsResources = 0;
    (*(int*)&AsCTopMenuBase(self)->textureResource) = 0;
    AsCTopMenuBase(self)->resultReady = 0;
    AsCTopMenuBase(self)->result = -1;
    AsCTopMenuBase(self)->bpEntryId = -1;
    AsCTopMenuBase(self)->backRequested = 0;
    AsCTopMenuBase(self)->scriptRunning = 0;
    void* p = new Message;
    AsCTopMenuBase(self)->message = p;
    AsCTopMenuBase(self)->optionDialogRequested = 0;
    if (archiveName) {
        void* archiveData = 0;
        if (!Sing_805DC620()) {
            if (!s_resourceTableInitialized) {
                s_resourceTableStorage = 0;
                __register_global_object(&s_resourceTableStorage, (void (*)())Allocation_Dtor, &s_resourceTableDtorRegistration_TopMenu);
                s_resourceTableInitialized = 1;
            }
            Sing_805DC620() = &s_resourceTableStorage;
        }
        int textureCount = ResourceTable_FindEntryByName(Sing_805DC620(), archiveName, &archiveData);
        p = new Texture(archiveName, archiveData, textureCount);
        AsCTopMenuBase(self)->textureResource = p;
    }
    p = new Layout(5);
    AsCTopMenuBase(self)->layout = p;
    Layout_LoadExtended(p, 0, layoutName);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_ClubSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), bpResourceName, (int*)((char*)&AsCTopMenuBase(self)->bpEntryId), 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_ClubSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    AsCTopMenuBase(self)->bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCTopMenuBase(self)->bpEntryId);
    SoundManager_RequestBgmChange(g_soundManager, ((TopMenuStringPool*)topMenuAudioStrings)->musicFile, 0, 1);
    return self;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Attach borrowed layout/BP resources and create this menu's own message helper.
int CTopMenuBase_InitMembers(void* self, int layoutAddress, void* bpObject, int itemCount, int elementIdTable, int unused0, int unused1) {
    (*(int*)&AsCTopMenuBase(self)->layout) = layoutAddress;
    AsCTopMenuBase(self)->vtbl = &__vt__12CTopMenuBase;
    AsCTopMenuBase(self)->bpObject = bpObject;
    AsCTopMenuBase(self)->menuItemCount = itemCount;
    (*(int*)&AsCTopMenuBase(self)->message) = 0;
    AsCTopMenuBase(self)->optionDialog = 0;
    AsCTopMenuBase(self)->elementIds = elementIdTable;
    AsCTopMenuBase(self)->borrowsResources = 1;
    (*(int*)&AsCTopMenuBase(self)->textureResource) = 0;
    AsCTopMenuBase(self)->resultReady = 0;
    AsCTopMenuBase(self)->result = -1;
    AsCTopMenuBase(self)->bpEntryId = -1;
    AsCTopMenuBase(self)->backRequested = 0;
    AsCTopMenuBase(self)->scriptRunning = 0;
    struct Obj12 { char data[12]; Obj12() { MenuMessage_Ctor(this); } };
    Obj12* p = new Obj12();
    AsCTopMenuBase(self)->message = p;
    AsCTopMenuBase(self)->optionDialogRequested = 0;
    return (int)self;
}

#pragma use_lmw_stmw on
// Select the first hovered item's message, using the final table entry as the fallback.
int CTopMenuBase_UpdateHoverMessage(void* self, int* indices) {
    MenuMessage_ResetTextState(AsCTopMenuBase(self)->message);
    int elementIndex;
    int found;
    int i;
    found = 0;
    i = 0;
    elementIndex = 1;
    while (i < AsCTopMenuBase(self)->menuItemCount) {
        bool hit = Layout_HitTestPlayer(AsCTopMenuBase(self)->layout, elementIndex) != 0;
        if (hit) {
            MenuMessage_SetText(AsCTopMenuBase(self)->message, indices[i]);
            found = 1;
            break;
        }
        elementIndex += kMenuItemElementStride;
        ++i;
    }
    if (!found) MenuMessage_SetText(AsCTopMenuBase(self)->message, indices[AsCTopMenuBase(self)->menuItemCount]);
    return 1;
}
#pragma use_lmw_stmw reset

#pragma opt_lifetimes off
#pragma opt_common_subs off
// Request the options dialog when its input action is pressed.
int CTopMenuBase_RequestOptionsDialog(void* self) {
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, InputManager_Dtor, (void*)s_inputManagerDtorRegistration_TopMenu);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    if (Input_IsActionPressed(Sing_805DC5C8(), kActionOpenOptions, kAllInputPlayers)) {
        AsCTopMenuBase(self)->optionDialogRequested = 1;
        return 1;
    }
    return 0;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
// Set the rendered message's position.
int MenuMessage_SetPosition(void* self, float* pos) {
    Text_SetPosition(AsMenuMessage802CE2BC(self)->text, pos);
    return 1;
}

// Advance the message's character reveal.
int MenuMessage_UpdateReveal(void* self) {
    CMsgDraw_UpdateReveal(AsMenuMessage802CE2BC(self)->text);
    return 1;
}

// Draw the message glyphs.
int MenuMessage_DrawText(void* self) {
    Text_DrawGlyphs(AsMenuMessage802CE2BC(self)->text);
    return 1;
}

// Set the rendered message's layout bounds.
int MenuMessage_SetLayoutSize(void* self, float* size) {
    CMsgDraw_SetLayoutSize(AsMenuMessage802CE2BC(self)->text, size);
    return 1;
}

// Return whether the requested layout player/element is hovered.
int CTopMenuBase_HitTestPlayer(void* self, int index) {
    int r = Layout_HitTestPlayer(AsCTopMenuBase(self)->layout, index);
    return r != 0;
}

// Switch each item's idle/hover pair within its four-element layout group.
int CTopMenuBase_UpdateHoverElementPairs(void* self) {
    int offset;
    int next;
    int i;
    offset = 0;
    i = 0;
    while (i < AsCTopMenuBase(self)->menuItemCount) {
        next = offset + 1;
        bool hit0 = Layout_HitTestPlayer(AsCTopMenuBase(self)->layout, offset) != 0;
        if (hit0) {
            Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, offset, 0);
            Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, next, 1);
        } else {
            bool hit1 = Layout_HitTestPlayer(AsCTopMenuBase(self)->layout, next) != 0;
            if (!hit1) {
                Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, offset, 1);
                Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, next, 0);
            }
        }
        offset += kMenuItemElementStride;
        ++i;
    }
    return 1;
}

// Reset reveal state when a rendered message exists.
int MenuMessage_ResetTextState(void* self) {
    void* p = AsMenuMessage802CE2BC(self)->text;
    if (p == 0) {
        return 0;
    }
    CMsgDraw_ResetTextState(p);
    return 1;
}

struct WideStringAllocatorByte { unsigned char c; };
// Wide-string storage and cleanup; retain the type name used by exception relocations.
struct Str802CE4C8 {
    int w[3];
    Str802CE4C8() { w[0] = 0; w[1] = 0; w[2] = 0; }
    ~Str802CE4C8() {
        WideStringAllocatorByte alloc;
        WideStringAllocatorByte copy = alloc;
        WString_ReplaceRange(w, 0, -1, 0, 0, (unsigned char*)&copy);
        if ((unsigned)w[0] >> 31) appFreeUni__FPv((void*)w[2]);
    }
};

// Clear text/reveal formatting, expand the selected message entry, and assign its wide text.
int MenuMessage_SetText(void* self, int messageIndex) {
    if (!AsMenuMessage802CE2BC(self)->text || !AsMenuMessage802CE2BC(self)->taggedText) return 0;
    Str802CE4C8 expandedText;
    CMsgDraw_ResetTextState(AsMenuMessage802CE2BC(self)->text);
    CMsgDraw_ClearFormattingAndRevealAll(AsMenuMessage802CE2BC(self)->text);
    Text_ExpandTaggedText(AsMenuMessage802CE2BC(self)->taggedText, expandedText.w, messageIndex, 0);
    Text_SetContent(AsMenuMessage802CE2BC(self)->text,
                !((unsigned)expandedText.w[0] >> 31) ? (void*)((char*)expandedText.w + 2) : (void*)expandedText.w[2]);
    return 1;
}

// Wait for the optional texture archive, layout, and all shared BP resources.
int CTopMenuBase_PollLoad(void* self) {
    void* p = AsCTopMenuBase(self)->textureResource;
    // An empty pending archive path marks completion, including a failed texture import.
    if (p && !CPackTextureResource_IsLoadFinished(p)) {
        CPackTextureResource_PollLoad(AsCTopMenuBase(self)->textureResource);
        return 0;
    }
    if (!Layout_PollLoadComplete(AsCTopMenuBase(self)->layout)) return 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_ClubSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    if (!CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0())) return 0;
    return 1;
}

extern "C" void* UIGroup_GetOrCreateById(void*, int);
extern "C" int UIGroup_SetEnabled(void*, int);
extern "C" void Layout_SetElementEnabled(void*, int, int);
extern "C" void Layout_SetElementValueState(void*, int, int);
extern "C" void UIGroup_GetPosition(void*, int*);
extern "C" void Layout_SetElementPosition(void*, int, int*);
extern "C" void Layout_SetElementPlayerFlagsAndState(void*, int, int);
extern "C" void WFixedFont_CreateMsgEcbResource(void*);
extern "C" void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
extern "C" ChatMessageFactoryView* g_chatMsg;

// Disable the BP group identified by the menu's element-id table.
int CTopMenuBase_HideItemGroup(void* self, int index) {
    int* arr = (*(int**)&AsCTopMenuBase(self)->elementIds);
    void* base = AsCTopMenuBase(self)->bpObject;
    int key = arr[index];
    void* r = UIGroup_GetOrCreateById(base, key);
    UIGroup_SetEnabled(r, 0);
    return 1;
}

// Copy a BP group's position into the corresponding layout element.
int CTopMenuBase_SyncItemPosition(void* self, int index) {
    int key = ((*(int**)&AsCTopMenuBase(self)->elementIds))[index];
    void* element = UIGroup_GetOrCreateById(AsCTopMenuBase(self)->bpObject, key);
    int pos[2] = {0, 0};
    UIGroup_GetPosition(element, pos);
    Layout_SetElementPosition(AsCTopMenuBase(self)->layout, index, pos);
    return 1;
}

// Clear the hover element state for every four-element menu item.
int CTopMenuBase_ResetItemPlayerStates(void* self) {
    if (AsCTopMenuBase(self)->layout == 0) return 0;
    int offset;
    int i;
    i = 0;
    offset = 1;
    while (i < AsCTopMenuBase(self)->menuItemCount) {
        Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, offset, 0);
        offset += kMenuItemElementStride;
        ++i;
    }
    return 1;
}

#pragma opt_common_subs off
// Create tagged-text and rendered-text objects from the shared font factory at the origin.
void* MenuMessage_Ctor(void* self) {
    int z;
    int pos[2];
    z = 0;
    (*(int*)&AsMenuMessage802CE2BC(self)->taggedText) = z;
    (*(int*)&AsMenuMessage802CE2BC(self)->textFactory) = z;
    (*(int*)&AsMenuMessage802CE2BC(self)->text) = z;
    AsMenuMessage802CE2BC(self)->textFactory = g_chatMsg->textFactory;
    ((void (*)(void*, void*, int, int))WFixedFont_CreateMsgEcbResource)(AsMenuMessage802CE2BC(self)->textFactory, self, 0, 0);
    WFixedFont_CreateAndRegisterMsgDraw(AsMenuMessage802CE2BC(self)->textFactory, (char*)&AsMenuMessage802CE2BC(self)->text, 1);
    pos[0] = z;
    pos[1] = z;
    ((void (*)(void*, int*))Text_SetPosition)(AsMenuMessage802CE2BC(self)->text, pos);
    return self;
}
#pragma opt_common_subs reset

// Unregister and destroy both text objects, then optionally free the message helper.
void* MenuMessage_Dtor(void* self, int destroy) {
    if (self) {
        if (!Sing_805DC650()) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor, s_messageDrawPairListDtorRegistration_TopMenu);
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), AsMenuMessage802CE2BC(self)->text);
        void* p = AsMenuMessage802CE2BC(self)->text;
        if (p) {
            if (p) {
                struct D { 
                virtual void d(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((D*)p)->d(1);
            }
            AsMenuMessage802CE2BC(self)->text = 0;
        }
        p = AsMenuMessage802CE2BC(self)->taggedText;
        if (p) {
            if (p) {
                struct D { 
                virtual void d(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((D*)p)->d(1);
            }
            AsMenuMessage802CE2BC(self)->taggedText = 0;
        }
        if (destroy > 0) __dl__FPv(self);
    }
    return self;
}

// Release owned resources unless borrowed, destroy message/options storage, and optionally free the menu.
void* __dt__7MPBase2Fv(void* self, int destroy) {
    if (self) {
        void* p;
        AsCTopMenuBase(self)->vtbl = (void*)__vt__12CTopMenuBase;
        unsigned char mode = AsCTopMenuBase(self)->borrowsResources;
        if (!mode) {
            p = AsCTopMenuBase(self)->layout;
            if (p) {
                Layout_Destroy(p, 1);
                AsCTopMenuBase(self)->layout = 0;
            }
            if (AsCTopMenuBase(self)->bpEntryId != -1) {
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) {
                        __ct__7CBpCtrlFv(&s_bpCtrl);
                        __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_ClubSelect);
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = &s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCTopMenuBase(self)->bpEntryId);
                AsCTopMenuBase(self)->bpEntryId = -1;
            }
            p = AsCTopMenuBase(self)->textureResource;
            if (p) {
                if (p) {
                    struct D { 
                    virtual void d(int) DECOMP_LOCAL_VIRTUAL;
                    };
                    ((D*)p)->d(1);
                }
                AsCTopMenuBase(self)->textureResource = 0;
            }
        }
        p = AsCTopMenuBase(self)->message;
        if (p) {
            MenuMessage_Dtor(p, 1);
            AsCTopMenuBase(self)->message = 0;
        }
        p = (*(void**)&AsCTopMenuBase(self)->optionDialog);
        if (p) {
            __dl__FPv(p);
            (*(void**)&AsCTopMenuBase(self)->optionDialog) = 0;
        }
        if (destroy > 0) __dl__FPv(self);
    }
    return self;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Return the first activated element; throw out_of_range if collection succeeds with an empty vector.
int CTopMenuBase_GetFirstActivatedElement(void* self) {
    // Activated-element vector; cleanup retains the original exception-handler symbol.
    struct Items802CDB58 {
        void* p;
        int n;
        void* q;
        Items802CDB58() { p = 0; n = 0; q = 0; }
        ~Items802CDB58() {
            if (p) {
                int t = *(volatile int*)&n;
                n = t - t;
                appFreeUni__FPv(p);
            }
        }
    };
    struct SharedErrorMessage {
        void* vt;
        int one;
        int two;
        char* msg;
        SharedErrorMessage(char* m) { one = 1; two = 1; vt = &s_sharedCharDeleterVtable; msg = m; }
    };
    Items802CDB58 items;
    if (!Layout_CollectActivated(AsCTopMenuBase(self)->layout, &items, 0)) {
        return -1;
    }
    if (items.n == 0) {
        // Stack-local out_of_range object; type spelling is part of exception cleanup relocations.
    struct Error802CDB58 {
            void* vtable;
            char* message;
            SharedErrorMessage* volatile repObj;
            ~Error802CDB58() {}
        };
        // Own the allocated error message until its shared representation accepts it.
    struct Pair802CDB58 {
            char* p;
            char* q;
            ~Pair802CDB58() { if (p) __dla__FPv(p); }
        };
        char* text = ((TopMenuStringPool*)topMenuAudioStrings)->emptySelectionError;
        Error802CDB58 error;
        Error802CDB58* ep = &error;
        char flag;
        error.vtable = &__vt__Q23std11logic_error;
        flag = 0;
        {
        Pair802CDB58 pair;
        unsigned int len = strlen(text) + 1;
        char* copy = (char*)__nwa__FUl(len * 2 - len);
        error.message = copy;
        pair.p = copy;
        pair.q = &flag;
        error.repObj = new SharedErrorMessage(copy);
        pair.p = 0;
        }
        strcpy(error.message, text);
        error.vtable = &__vt__Q23std12out_of_range;
        __throw(((TopMenuStringPool*)topMenuAudioStrings)->outOfRangeType, ep, (void*)__dt__Q23std12out_of_rangeFv);
    }
    return *(int*)items.p;
}
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset

// Destroy and clear the optional owned texture archive.
void CTopMenuBase_ReleaseTextureResource(void* self) {
    if (AsCTopMenuBase(self)->textureResource != 0) {
        if (AsCTopMenuBase(self)->textureResource != 0) {
            struct CE { 
            virtual void c(int) DECOMP_LOCAL_VIRTUAL;
            };
            CE* p = (CE*)AsCTopMenuBase(self)->textureResource;
            p->c(1);
        }
        AsCTopMenuBase(self)->textureResource = 0;
    }
}

// Enable the preceding layout element, disable the current one, and set pressed feedback.
int CTopMenuBase_EnablePreviousElement(void* menu, int index) {
    int idx = index;
    void* self = menu;
    Layout_SetElementEnabled(AsCTopMenuBase(self)->layout, idx - 1, 1);
    Layout_SetElementEnabled(AsCTopMenuBase(self)->layout, idx, 0);
    Layout_SetElementValueState(AsCTopMenuBase(self)->layout, idx - 1, 2);
    return 1;
}

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_propagation reset
#pragma opt_common_subs off
// Flush one or two circular-window spans to output, updating byte counts, checksum, and buffer-error status.
int Zlib_InflateFlushWindow(void* window, void* output, int status) {
    register unsigned char* destination;
    register unsigned char* readCursor;
    unsigned size;
    unsigned char* writeCursor;
    readCursor = AsInflateWindow802CE62C(window)->readCursor;
    writeCursor = AsInflateWindow802CE62C(window)->writeCursor;
    destination = AsInflateOutput802CE62C(output)->nextOutput;
    size = (readCursor <= writeCursor ? writeCursor : AsInflateWindow802CE62C(window)->windowEnd) - readCursor;
    unsigned availableOutput = AsInflateOutput802CE62C(output)->availableOutput;
    if (size > availableOutput) size = availableOutput;
    if (size != 0) { if (status == kZlibBufferError) status = kZlibOk; }
    AsInflateOutput802CE62C(output)->availableOutput -= size;
    AsInflateOutput802CE62C(output)->totalOutput += size;
    if (AsInflateWindow802CE62C(window)->checksumCallback) {
        typedef void* (*ChecksumCallback)(void*, unsigned char*, unsigned);
        void* checksum = ((ChecksumCallback)AsInflateWindow802CE62C(window)->checksumCallback)(AsInflateWindow802CE62C(window)->checksum, readCursor, size);
        AsInflateWindow802CE62C(window)->checksum = checksum;
        AsInflateOutput802CE62C(output)->checksum = checksum;
    }
    if (size != 0) {
        memcpy(destination, (void*)readCursor, size);
        destination += size;
        readCursor += size;
    }
    if (readCursor == AsInflateWindow802CE62C(window)->windowEnd) {
        unsigned writeAddress = (*(unsigned*)&AsInflateWindow802CE62C(window)->writeCursor);
        readCursor = AsInflateWindow802CE62C(window)->windowBegin;
        if (writeAddress == (*(unsigned*)&AsInflateWindow802CE62C(window)->windowEnd)) AsInflateWindow802CE62C(window)->writeCursor = readCursor;
        size = AsInflateWindow802CE62C(window)->writeCursor - readCursor;
        unsigned remainingOutput = AsInflateOutput802CE62C(output)->availableOutput;
        if (size > remainingOutput) size = remainingOutput;
        if (size != 0) { if (status == kZlibBufferError) status = kZlibOk; }
        AsInflateOutput802CE62C(output)->availableOutput -= size;
        AsInflateOutput802CE62C(output)->totalOutput += size;
        if (AsInflateWindow802CE62C(window)->checksumCallback) {
            typedef void* (*ChecksumCallback)(void*, unsigned char*, unsigned);
            void* checksum = ((ChecksumCallback)AsInflateWindow802CE62C(window)->checksumCallback)(AsInflateWindow802CE62C(window)->checksum, readCursor, size);
            AsInflateWindow802CE62C(window)->checksum = checksum;
            AsInflateOutput802CE62C(output)->checksum = checksum;
        }
        if (size != 0) {
            memcpy(destination, (void*)readCursor, size);
            destination += size;
            readCursor += size;
        }
    }
    AsInflateOutput802CE62C(output)->nextOutput = destination;
    AsInflateWindow802CE62C(window)->readCursor = readCursor;
    return status;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation off
#pragma use_lmw_stmw reset

// Wait for fading, process options/cancel, then dispatch menu selection input.
int CTopMenuBase_ProcessInput(void* self) {
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, &s_fadeOverlayDtorRegistration_TopMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608())) {
        return 0;
    }
    struct TopMenuInputCalls {
        virtual int unusedSlot0() DECOMP_LOCAL_VIRTUAL;
        virtual int handleOptions() DECOMP_LOCAL_VIRTUAL;
        virtual int handleSelection() DECOMP_LOCAL_VIRTUAL;
    };
    if (((TopMenuInputCalls*)self)->handleOptions() != 0) return 1;
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, InputManager_Dtor, &s_inputManagerDtorRegistration_TopMenu);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
    bool handled;
    if (Input_IsActionPressed(Sing_805DC5C8(), kActionCancel, kAllInputPlayers)) {
        BpObject_StartScriptByName(AsCTopMenuBase(self)->bpObject, (void*)((TopMenuStringPool*)topMenuAudioStrings)->exitScript, 1);
        AsCTopMenuBase(self)->backRequested = 1;
        int itemIndex = 0;
        int idleElement = 0;
        int hoverElement = 0;
        // Base +0x18 aliases derived menus' scriptRunning at +0x30; wait for the exit script.
        AsCTopMenuBase(self)->scriptRunning = 1;
        while (itemIndex < AsCTopMenuBase(self)->menuItemCount) {
            hoverElement = idleElement + 1;
            Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, idleElement, 1);
            Layout_SetElementPlayerFlagsAndState(AsCTopMenuBase(self)->layout, hoverElement, 0);
            idleElement += kMenuItemElementStride;
            ++itemIndex;
        }
        handled = true;
    } else {
        handled = false;
    }
    if (handled) return 1;
    return ((TopMenuInputCalls*)self)->handleSelection();
}
#pragma opt_propagation reset
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624F4", _ctors_u802CD608_0, MenuMessage_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804624F4", _ctors_u802CD608_1, InflateWindow_InitTauConstant);
#pragma pop

