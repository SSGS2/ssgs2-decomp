#include <decomp/static_init.h>
extern "C" char s_partyBpHandleDestructorRegistration[];
// CChatMsg's singleton prefix precedes its fixed-font resource.
struct ChatMessageResourceView { char singletonBase[0x10]; void* resource; };
#include "classes/WFixedFont.h"
// UTF-16BE message templates, exposed as byte spans to preserve the existing expansion ABI.
struct ItemRewardMessageStrings {
    char getCourse[54]; // +0x0: "\\<D=MSG_COMMON_GET_COURSE>"
    char getCharacter[60]; // +0x36: "\\<D=MSG_COMMON_GET_CHARACTER>"
    char getItem[50]; // +0x72: "\\<D=MSG_COMMON_GET_ITEM>"
    char getCostumeSet[62]; // +0xA4: "\\<D=MSG_COMMON_GET_COSTUMESET>"
    char getCostume[56]; // +0xE2: "\\<D=MSG_COMMON_GET_COSTUME>"
    char getCompleteItem[68]; // +0x11A: "\\<D=MSG_COMMON_GET_COMPLETE_ITEM>"
    char releaseCaddie[62]; // +0x15E: "\\<D=MSG_COMMON_RELEASE_CADDIE>"
    char releaseCostumeSet[70]; // +0x19C: "\\<D=MSG_COMMON_RELEASE_COSTUMESET>"
    char releaseCostume[64]; // +0x1E2: "\\<D=MSG_COMMON_RELEASE_COSTUME>"; also used for clubs, balls and caddie items.
};

struct LoadingDrawStrings {
    char courseArchive[11]; // +0x0: "course.zip"
    char loadingLayout[12]; // +0xB: "loading.b2d"
};

struct PopItemDialogStrings {
    char setDialogScript[15]; // +0x0: "pop_set_dialog"
    char itemDialogScript[16]; // +0xF: "pop_item_dialog"
    char itemBaseGroup[15]; // +0x1F: "R_par_itembase"
    char courseBaseGroup[17]; // +0x2E: "R_par_coursebase"
    char confirmSound[19]; // +0x3F: "ui_button_ok_click"
    char cancelSound[23]; // +0x52: "ui_button_cancel_click"
    char courseIconGroup[12]; // +0x69: "course_icon"
    char itemIconGroup[10]; // +0x75: "item_icon"
    char textureArchive[16]; // +0x7F: "item_dialog.zip"
    char textureSuffix[5]; // +0x8F: ".gsr"
    char setLayout[18]; // +0x94: "p2_set_dialog.b2d"
    char itemLayout[19]; // +0xA6: "p2_item_dialog.b2d"
    char messageFile[12]; // +0xB9: "iff_msg.eky"
    char setNameGroup[21]; // +0xC5: "R_MSG_setitem_K16_LL"
    char itemNameGroup[18]; // +0xDA: "R_MSG_item_K16_LL"
    char setItemGroupFormat[23]; // +0xEC: "R_MSG_setitem%d_K16_LL"
    char detailGroup[20]; // +0x103: "R_MSG_detail_K30_LL"
};

#include "iff/IffStruct.h"
#include "classes/CMsgDraw.h"
#include "classes/CBpCtrlHandle.h"
#define AsCBpCtrlHandle(p) ((CBpCtrlHandle*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

// Translation unit 0x802861B0-0x802877E8 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadingDraw.h"
#include "classes/CPopItemDialog.h"
#include "classes/CFileIoAccessCheckFlash.h"
#include "classes/CFileIoAccessCheck.h"

#define AsCFileIoAccessCheck(p) ((CFileIoAccessCheck*)(p))

#define AsCFileIoAccessCheckFlash(p) ((CFileIoAccessCheckFlash*)(p))

#define AsCPopItemDialog(p) ((CPopItemDialog*)(p))

#define AsCLoadingDraw(p) ((CLoadingDraw*)(p))
struct PopItemRegistrations {
    char bpCtrl[12];
    char bpHandle[12];
    char inputManager[12];
    char resourceTable[12];
    char messageFileName[12];
};
struct PopItemSavedScript {
    char dialogState[0x2C];
    int previousScriptIndex;
};
// Access-check state map: 0 starts the request; 1 polls device busy; 2 records
// failure; 3 is terminal. A missing resource enters failure, not successful ready.
// Dialog map: 0 waits/creates text, 1 delays one update, 2 accepts input, 3 returns
// completion. Confirm/cancel only set state 3; completion is reported next call.
// Layout groups: itemNameGroup/setNameGroup = heading; setItemGroupFormat uses
// one-based member numbers; detailGroup = expanded description; icon groups are
// itemIconGroup/courseIconGroup. The opening script hides the OTHER base group.
// descriptionMode: 0 acquisition, 1 release, 2 completion; only listed categories
// produce text. Other mode/category combinations leave the cleared text empty.
enum FileAccessCheckState {
    kCheckStart = 0, kCheckPending = 1, kCheckRecordFailure = 2, kCheckFinished = 3
};
enum PopDialogLifecycle {
    kDialogWaitingForResources = 0, kDialogOpening = 1, kDialogWaitingForInput = 2, kDialogFinished = 3
};
// Word access in the lifecycle hook preserves the aliasing of the original ABI view.
enum PopDialogWord {
    kDialogItemIdWord = 1, kDialogBpEntryWord = 2, kDialogStateWord = 3,
    kDialogTextWord = 6, kDialogTaggedTextWord = 7, kDialogFileWord = 9,
    kDialogTexturesWord = 10, kDialogSavedScriptWord = 11
};
enum DescriptionMode { kDescriptionAcquisition = 0, kDescriptionRelease = 1, kDescriptionCompletion = 2 };
enum DialogConstants { kItemCategoryShift = 26, kItemSetCategory = 9, kCourseCategory = 10,
       kCharacterCategory = 1, kCostumeCategory = 2, kClubSetCategory = 4,
       kBallCategory = 5, kCaddieCategory = 7, kCaddieItemCategory = 8, kAuxPartCategory = 28,
       kMenuConfirmAction = 4, kMenuCancelAction = 6, kLoadingCourseCount = 14 };

extern "C" {
extern char s_bpCtrlDtorRegistration_LoadingDraw[];
extern char s_loadingDrawStringStorage[];

extern float s_twoPi_FileIoAccessCheck;
extern float s_twoPi_PopItemDialog;
extern float s_twoPi_LoadingDraw;
extern const float kFloat_6_2831855;
extern char __vt__18CFileIoAccessCheck[];
extern char __vt__12CLoadingDraw[];
extern char loadingDrawStrings[];
extern void* s_loadingScreenBackgroundsByCourse[kLoadingCourseCount]; // Per-course loading-screen background names.
extern char s_bpCtrl[];
extern char popItemDialogStrings[];
extern float kPopItemIconQuadTemplate[];
extern char s_inputManagerDtorRegistration_LoadingDraw[];
extern signed char s_bpCtrlInitialized;
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern void *s_loadingDrawRequiresConfirmation;
extern void* g_sharedDoc;
extern char s_bpCtrlDtorRegistration_PopItem[];
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor();
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern int CSharedFilePtr_GetLoadedBuffer(void*);
extern int CPackTextureResource_IsLoadFinished(void*);
extern void* IdMap_Lookup(void*, void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern void CMsgDraw_UpdateReveal(void*);
extern void Text_DrawGlyphs(void*);
extern void* BpObject_GetOrCreateNamedGroup(void*, char*);
extern void UIGroup_GetPosition(void*, void*);
extern void UIGroup_GetSourceSize(void*, void*);
extern int GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void* MPBase1_Ctor(void*);
extern int ResourceTable_FindEntryByName(void*, char*, unsigned int*);
extern void* CPackTextureResource_Init(void*, char*, unsigned int, void*);
extern int SharedFile_Create(void*, int);
extern void* ItemInventory_FindDataWithHairStyleFallback(void*, unsigned int);
extern void CBpTex_Init(void*);
extern void CBpCtrl_CreateEntry(void*, char*, void*, int);
extern int ItemInventory_FindCourseIndex(void*, void*);
extern void Allocation_Dtor();
extern void CFileIoAccess_Ctor(void*);
extern void* destroy__13CFileIoAccessFi(void*, int);
extern void __dl__FPv(void*);
extern void* CFileIoFlash_CreateInstance(void*, void*);
extern void* g_chatMsg;
extern signed char s_bpCtrlHandleInitialized;
extern void* s_bpCtrlHandle;
extern signed char s_inputManagerInitialized;
extern void* g_soundManager;
extern float kFloat_0_5_PopItemDialogSound;
extern float kFloat_200_PopItemDialog;
extern float kFloat_1_PopItemDialog;
extern char s_gameInputManager[];
extern void BpObject_StartScriptByName(void*, char*, int);
extern void* CMsgDraw_Ctor(void*, int);
extern void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
extern void UIGroup_SetEnabled(void*, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern int Input_IsActionPressed(void*, int, int);
extern void Sound_PlayByName(void*, char*, int, int, int, int, float, float, float);
extern void CBpCtrlHandle_ReleaseEntry(void*);
extern void CPackTextureResource_PollLoad(void*);
extern void appFreeUni__FPv(void*);
extern void CBpCtrlHandle_Dtor();
extern char* strcpy(char*, const char*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, void*, void*, void*);
extern void NString_AssignCString(void*, char*);

extern void* CBpCtrl_DeleteEntryById(void*, int);
extern int CSharedFileHandle_Destroy(void*, int);

// This access-check lifecycle hook is immediately ready.
int CFileIoAccessCheck_vfunc_2() {
    return 1;
}

// Initialize the access-check helper's two-pi cache.
void FileIoAccessCheck_InitTwoPiCache() {
    s_twoPi_FileIoAccessCheck = kFloat_6_2831855;
}

// Initialize the item-dialog helper's two-pi cache.
void PopItemDialog_InitTwoPiCache() {
    s_twoPi_PopItemDialog = kFloat_6_2831855;
}

// Initialize the loading-screen helper's two-pi cache.
void LoadingDraw_InitTwoPiCache() {
    s_twoPi_LoadingDraw = kFloat_6_2831855;
}

// Report whether the access-check state machine has reached its terminal state.
int CFileIoAccessCheck_IsFinished(void* a) {
    int v = AsCFileIoAccessCheckFlash(a)->state;
    return v == kCheckFinished;
}

// Create the flash device through the common file-I/O factory.
void* CFileIoAccessCheckFlash_CreateFlash(void* a, void* b, void* c) {
    return CFileIoFlash_CreateInstance(b, c);
}

#pragma dont_inline on
// Construct the access base and initialize an empty block-check list.
void* CFileIoAccessCheck_Ctor(void* p) {
    CFileIoAccess_Ctor(p);
    AsCFileIoAccessCheck(p)->checkState = 0;
    *(void**)p = (void*)__vt__18CFileIoAccessCheck;
    AsCFileIoAccessCheck(p)->checkItems = 0;
    AsCFileIoAccessCheck(p)->checkItemCount = 0;
    return p;
}
#pragma dont_inline off

#pragma dont_inline on
// Release the block-check allocation, destroy the access base, and optionally delete the object.
void* CFileIoAccessCheck_dtor(void* p, int flag) {
    if (p != 0) {
        void* o10 = (*(void**)&AsCFileIoAccessCheck(p)->checkItems);
        *(void**)p = (void*)__vt__18CFileIoAccessCheck;
        if (o10 != 0) {
            __dl__FPv(o10);
            AsCFileIoAccessCheck(p)->checkItems = 0;
        }
        destroy__13CFileIoAccessFi(p, 0);
        if (flag > 0)
            __dl__FPv(p);
    }
    return p;
}
#pragma dont_inline off

// Destroy the shared access-check state and optionally delete the flash checker.
void* CFileIoAccessCheckFlash_dtor(void* p, int flag) {
    if (p != 0) {
        CFileIoAccessCheck_dtor(p, 0);
        if (flag > 0)
            __dl__FPv(p);
    }
    return p;
}

struct Obj4BC {
    char data[0x4bc];
    // Construct the owned text renderer for the fixed-font message resource.
    Obj4BC(int x) { CMsgDraw_Ctor(this, x); }
};
struct VtObj2 {
    virtual void v0(int);
};
struct PopSelf {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual int v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
};

// Convert a UI group's tint word to the text renderer's color word: swap the byte order and force alpha to 0xFF.
static inline unsigned int GroupTintToTextColor(unsigned int tint) {
    return (((tint & 0xFFu) << 24) | ((tint & 0xFF00u) << 8)) | (((tint & 0xFF0000u) >> 8) | 0xFFu);
}

// Lazily construct the global CBpCtrl on first use and publish it through the shared singleton slot.
static inline void BpCtrl_EnsureInstance() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDtorRegistration_PopItem);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Poll dialog resources, construct text, then wait for confirm/cancel before reporting completion.
int CPopItemDialog_UpdateUntilDismissed(void* self) {
    int* f = (int*)self;
    void* base = s_bpCtrlDtorRegistration_PopItem;
    struct LocalStr { unsigned int flags; unsigned int size; void* data;
        // Release a heap-backed temporary string; inline bytes require no cleanup.
        ~LocalStr(){ if (flags >> 31) appFreeUni__FPv(data); }
    };
    LocalStr ls; ls.flags = 0; ls.size = 0; ls.data = 0;
    (void)ls;
    // Phase 1: wait until resources can be consumed; no input is accepted yet.
    switch (f[kDialogStateWord]) {
    case kDialogWaitingForResources: {
        if (((PopSelf*)self)->v3() != 0) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, base);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            void* dialogSheet = IdMap_Lookup(Sing_805DC5F0(), (void*)f[kDialogBpEntryWord]);
            char* name = ((PopItemDialogStrings*)popItemDialogStrings)->setDialogScript;
            if (!AsCPopItemDialog(self)->isItemSet) name = ((PopItemDialogStrings*)popItemDialogStrings)->itemDialogScript;
            BpObject_StartScriptByName(dialogSheet, name, 1);

            // Replace owned renderers only after the layout script is available.
            void* oldTextRenderer = (void*)f[kDialogTextWord];
            if (oldTextRenderer) {
                if (oldTextRenderer) {
                    ((VtObj2*)oldTextRenderer)->v0(1);
                }
                f[kDialogTextWord] = 0;
            }
            void* oldTaggedText = (void*)f[kDialogTaggedTextWord];
            if (oldTaggedText) {
                if (oldTaggedText) {
                    ((VtObj2*)oldTaggedText)->v0(1);
                }
                f[kDialogTaggedTextWord] = 0;
            }

            // Build message resources from loaded bytes, then populate all text groups.
            void* mgr = g_chatMsg;
            void* fixedFont = ((ChatMessageResourceView*)mgr)->resource;
            int messageBuffer = CSharedFilePtr_GetLoadedBuffer((void*)f[kDialogFileWord]);
            WFixedFont_CreateMsgEcbResource(fixedFont, &AsCPopItemDialog(self)->taggedText, messageBuffer, 1); // original +0x1C

            Obj4BC* textRenderer = new Obj4BC((int)AsWFixedFont(fixedFont)->messageResource);
            f[kDialogTextWord] = (int)textRenderer;

            ((PopSelf*)self)->v5();

            if (!AsCPopItemDialog(self)->isItemSet) {
                unsigned int x = AsCPopItemDialog(self)->itemId >> kItemCategoryShift;
                char* name2 = ((PopItemDialogStrings*)popItemDialogStrings)->courseBaseGroup;
                name2 = (x == kCourseCategory) ? ((PopItemDialogStrings*)popItemDialogStrings)->itemBaseGroup : name2;
                void* st = BpObject_GetOrCreateNamedGroup(dialogSheet, name2);
                UIGroup_SetEnabled(st, 0);
            }

            if (!Sing_805DC5F8()) {
                if (!s_bpCtrlHandleInitialized) {
                    void** o = &s_bpCtrlHandle;
                    ((CBpCtrlHandle*)o)->scriptIndex = -1;
                    *o = (void*)(long)-1;
                    __register_global_object(o, (void*)CBpCtrlHandle_Dtor, (((PopItemRegistrations*)base)->bpHandle));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = &s_bpCtrlHandle;
            }
            // Save the previous script selection before releasing its shared handle.
            void* g = Sing_805DC5F8();
            f[kDialogSavedScriptWord] = AsCBpCtrlHandle(g)->scriptIndex;
            if (!g) {
                if (!s_bpCtrlHandleInitialized) {
                    void** o = &s_bpCtrlHandle;
                    ((CBpCtrlHandle*)o)->scriptIndex = -1;
                    *o = (void*)(long)-1;
                    __register_global_object(o, (void*)CBpCtrlHandle_Dtor, (((PopItemRegistrations*)base)->bpHandle));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = &s_bpCtrlHandle;
            }
            g = Sing_805DC5F8();
            CBpCtrlHandle_ReleaseEntry(g);

            void* oldTexturePack = (void*)f[kDialogTexturesWord];
            if (oldTexturePack) {
                if (oldTexturePack) {
                    ((VtObj2*)oldTexturePack)->v0(1);
                }
                f[kDialogTexturesWord] = 0;
            }
            f[kDialogStateWord] = kDialogOpening;
        } else {
            void* texturePack = (void*)f[kDialogTexturesWord];
            if (texturePack) CPackTextureResource_PollLoad(texturePack);
        }
        break;
    }
    case kDialogOpening:
        f[kDialogStateWord] = kDialogWaitingForInput;
        break;
    // Phase 2: either action closes the dialog; sound is the only difference.
    case kDialogWaitingForInput: {
        if (!Sing_805DC5C8()) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, (((PopItemRegistrations*)base)->inputManager));
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = s_gameInputManager;
        }
        int pressed = Input_IsActionPressed(Sing_805DC5C8(), kMenuConfirmAction, -1);
        if (pressed) {
            Sound_PlayByName(g_soundManager, ((PopItemDialogStrings*)popItemDialogStrings)->confirmSound, 0, 0, 0, 0, kFloat_0_5_PopItemDialogSound, kFloat_200_PopItemDialog, kFloat_1_PopItemDialog);
            f[kDialogStateWord] = kDialogFinished;
        } else {
            if (!Sing_805DC5C8()) {
                if (!s_inputManagerInitialized) {
                    InputManager_Init(s_gameInputManager);
                    __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, (((PopItemRegistrations*)base)->inputManager));
                    s_inputManagerInitialized = 1;
                }
                Sing_805DC5C8() = s_gameInputManager;
            }
            pressed = Input_IsActionPressed(Sing_805DC5C8(), kMenuCancelAction, -1);
            if (pressed) {
                Sound_PlayByName(g_soundManager, ((PopItemDialogStrings*)popItemDialogStrings)->cancelSound, 0, 0, 0, 0, kFloat_0_5_PopItemDialogSound, kFloat_200_PopItemDialog, kFloat_1_PopItemDialog);
                f[kDialogStateWord] = kDialogFinished;
            }
        }
        break;
    }
    case kDialogFinished:
        return 1;
    default:
        break;
    }
    return 0;
}

extern void* __nwa__FUl(unsigned int);
extern int CFileIoAccess_CreateResource(void*, void*, void*);
extern int CFileIoAccess_GetResource(void*);
extern int CFileIoAccess_SetFailureFlag(char*, int);
extern int CFileIo_IsBusy(void*);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern char __vt__23CFileIoAccessCheckFlash[];

#pragma dont_inline on
#pragma use_lmw_stmw on
// src borrows count 32-bit block values for the copy; a/b pass unchanged to base setup.
// A failed base setup can leave the new owned list installed. Returns the base result.
// Copy a nonempty block list into owned storage and configure the file-access request.
int CFileIoAccessCheckFlash_SetupWithBlockList(void* self, void* a, void* b, void* src, unsigned int count) {
    int* sw = (int*)self;
    if (src == 0 || count == 0) return 0;
    if (AsCFileIoAccessCheckFlash(self)->checkItems != 0) {
        __dla__FPv(AsCFileIoAccessCheckFlash(self)->checkItems);
        AsCFileIoAccessCheckFlash(self)->checkItems = 0;
    }
    AsCFileIoAccessCheckFlash(self)->checkItemCount = 0;
    AsCFileIoAccessCheckFlash(self)->checkItems = __nwa__FUl(count * 4);
    if (AsCFileIoAccessCheckFlash(self)->checkItems == 0) return 0;
    AsCFileIoAccessCheckFlash(self)->checkItemCount = count;
    Memory_Copy(AsCFileIoAccessCheckFlash(self)->checkItems, src, count << 2);
    return CFileIoAccess_CreateResource(self, a, b);
}
#pragma use_lmw_stmw reset
#pragma dont_inline off

#pragma dont_inline on
struct Obj54F8 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3(int);
};

// count must be positive and src must hold count 32-bit values. Success transfers
// the newly allocated checker to the caller; failure destroys it and returns null.
// Construct a flash checker and discard it if block-list/access setup fails.
void* CFileIoAccessCheckFlash_Create(void* a, void* b, void* src, int count) {
    void* obj = __nw__FUl(0x18);
    void* saved = obj;
    if (obj != 0) {
        CFileIoAccessCheck_Ctor(obj);
        *(void**)obj = (void*)__vt__23CFileIoAccessCheckFlash;
    }
    if (saved == 0) return 0;
    if (!CFileIoAccessCheckFlash_SetupWithBlockList(saved, a, b, src, count)) {
        if (saved != 0) {
            ((Obj54F8*)saved)->v3(1);
        }
        return 0;
    }
    return saved;
}
#pragma dont_inline off

struct PaneObj {
    virtual void p0();
    virtual void p1(void*);
};

struct VtObjA {
    virtual void a0();
    virtual void a1();
    virtual void a2();
    virtual void a3();
    virtual int a6(void*, int);
};

struct VtObjB {
    virtual void b0();
    virtual void b1();
    virtual void b2();
    virtual void b3();
    virtual int b6();
};

// Start/poll a block check; unavailable or rejected requests latch the failure flag.
// Always returns 1 (update handled); IsFinished, not this return, tests completion.
int CFileIoAccessCheck_Update(void* self) {
    void* obj = (void*)(long)CFileIoAccess_GetResource(self);
    if (obj == 0) {
        AsCFileIoAccessCheckFlash(self)->state = kCheckRecordFailure;
    }
    int state = AsCFileIoAccessCheckFlash(self)->state;
    switch (state) {
        case kCheckStart: {
            if (((VtObjA*)obj)->a6(AsCFileIoAccessCheckFlash(self)->checkItems,
                                   AsCFileIoAccessCheckFlash(self)->checkItemCount) == 0)
                AsCFileIoAccessCheckFlash(self)->state = kCheckRecordFailure;
            else
                AsCFileIoAccessCheckFlash(self)->state = kCheckPending;
            break;
        }
        case kCheckPending: {
            if (CFileIo_IsBusy(obj) == 0) AsCFileIoAccessCheckFlash(self)->state = kCheckFinished;
            break;
        }
        case kCheckRecordFailure: {
            CFileIoAccess_SetFailureFlag((char*)self, 1);
            AsCFileIoAccessCheckFlash(self)->state = kCheckFinished;
            break;
        }
        default:
            break;
    }
    return 1;
}

// Report readiness only after controller entries, message file, icon pane, and texture pack are ready.
int CPopItemDialog_AreResourcesLoaded(void* self) {
    int ok = 1;
    BpCtrl_EnsureInstance();
    if (!CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0())) ok = 0;
    if (!CSharedFilePtr_GetLoadedBuffer(AsCPopItemDialog(self)->sharedFile)) ok = 0;
    void* obj = AsCPopItemDialog(self)->texturePane;
    if (!((VtObjB*)obj)->b6()) ok = 0;
    obj = AsCPopItemDialog(self)->texturePack;
    if (obj && !CPackTextureResource_IsLoadFinished(obj)) ok = 0;
    return ok;
}

struct PopVertices {
    float data[24];
};
struct PopDrawObject {
    virtual void d0();
    virtual void d1();
    virtual void d2();
    virtual void d3();
    virtual void d4();
    virtual void d5();
};

// Icon corners use layout coordinates: (left,top) to (left+width,top+height).
// Each 24-byte template vertex supplies untouched depth/color/UV components;
// only XY is replaced. Primitive 0x98 is GX_TRIANGLESTRIP with four vertices.
// Draw the dialog animation, text, and item/course icon inside its named layout group.
void CPopItemDialog_DrawDialogTextAndIcon(const void* self) {
    if (AsCPopItemDialog(self)->state == 0) return;
    BpCtrl_EnsureInstance();
    void* s = IdMap_Lookup(Sing_805DC5F0(), AsCPopItemDialog(self)->bpEntryId);
    void* entry;
    BpObject_TryInvokeCurrentCallback(s, 0);
    BpObject_Draw(s);
    CMsgDraw_UpdateReveal(AsCPopItemDialog(self)->textRenderer);
    int zero = 0;
    ((CMsgDraw*)AsCPopItemDialog(self)->textRenderer)->drawSuppressed = zero;
    Text_DrawGlyphs(AsCPopItemDialog(self)->textRenderer);
    bool isCourse = (AsCPopItemDialog(self)->itemId >> kItemCategoryShift) == kCourseCategory;
    float groupPosition[2] = {0,0}, groupSize[2] = {0,0};
    entry = BpObject_GetOrCreateNamedGroup(s, isCourse ? ((PopItemDialogStrings*)popItemDialogStrings)->courseIconGroup : ((PopItemDialogStrings*)popItemDialogStrings)->itemIconGroup);
    UIGroup_GetPosition(entry, groupPosition);
    UIGroup_GetSourceSize(entry, groupSize);
    {
        void* device = AsCPopItemDialog(self)->texturePane;
        ((PopDrawObject*)device)->d3();
    }
    PopVertices verts = *(const PopVertices*)kPopItemIconQuadTemplate;
    float top;
    float left = groupPosition[0];
    top = groupPosition[1];
    float right = left + groupSize[0];
    float bottom = top + groupSize[1];
    verts.data[0] = left; verts.data[1] = top;
    verts.data[6] = right; verts.data[7] = top;
    verts.data[12] = left; verts.data[13] = bottom;
    verts.data[18] = right; verts.data[19] = bottom;
    int count = GX_GetPrimitiveCount(4, 0x98);
    DrawPrimitiveUP(0x98, count, &verts, 0x18);
}

struct LoadingName {
    unsigned int flags;
    unsigned int size;
    char* data;
    // Return borrowed inline/heap narrow string bytes; no ownership transfer.
    char* chars() {
        if ((flags >> 31) == 0) return (char*)this + 1;
        return data;
    }
};
struct LoadingStatics {
    char inputRegistrations[0x18];
    char bpCtrlRegistration[12];
    char resourceTableRegistration[12];
    char messageFileName[12];
    LoadingName messagePath;
};
struct LoadingItem20 {
    char data[0x20];
    // Construct the owned loading-screen texture pack from borrowed archive descriptors.
    LoadingItem20(char* name, unsigned int textureNamesAddress, int textureCount) {
        CPackTextureResource_Init(this, name, textureNamesAddress, (void*)textureCount);
    }
};
struct LoadingPane28 {
    char data[0x28];
    // Initialize the owned loading-screen background texture.
    LoadingPane28() { CBpTex_Init(this); }
};
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Load the loading-screen textures, optional message file, and background for the selected course.
void* CLoadingDraw_Ctor(void* self) {
    char* base = s_inputManagerDtorRegistration_LoadingDraw;
    int courseIndex;
    void* texturePack;
    void* backgroundPane;
    int textureCount;
    MPBase1_Ctor(self);
    *(void**)self = __vt__12CLoadingDraw;
    int* sw = (int*)self;
    AsCLoadingDraw(self)->messageFile=0; AsCLoadingDraw(self)->messageEcbResource=0;
    AsCLoadingDraw(self)->bpEntryId=-1; AsCLoadingDraw(self)->backgroundPane=0;
    AsCLoadingDraw(self)->messageDraw=0; AsCLoadingDraw(self)->texturePack=0;
    unsigned int textureNamesAddress=0;
    if (!Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage=0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (((LoadingStatics*)base)->resourceTableRegistration));
            s_resourceTableInitialized=1;
        }
        Sing_805DC620()=&s_resourceTableStorage;
    }
    // Request archive textures first; the optional message and course background follow.
    textureCount=ResourceTable_FindEntryByName(Sing_805DC620(), ((LoadingDrawStrings*)loadingDrawStrings)->courseArchive, &textureNamesAddress);
    texturePack=new LoadingItem20(((LoadingDrawStrings*)loadingDrawStrings)->courseArchive,textureNamesAddress,textureCount);
    AsCLoadingDraw(self)->texturePack=texturePack;
    if(s_loadingDrawRequiresConfirmation) {
        char* p=((LoadingName*)(&((LoadingStatics*)base)->messagePath))->chars();
        AsCLoadingDraw(self)->messageFile= (void*)(long)SharedFile_Create(p,0);
    }
    void* sys=g_sharedDoc;
    courseIndex=ItemInventory_FindCourseIndex(AsCSharedDoc(sys)->itemInventory, (void*)((IFF_STRUCT::sCourse*)AsCSharedDoc(sys)->selectedCourseData)->common.typeId);
    if(courseIndex>=kLoadingCourseCount || courseIndex<0)courseIndex=0;
    backgroundPane=new LoadingPane28;
    AsCLoadingDraw(self)->backgroundPane=backgroundPane;
    ((PaneObj*)backgroundPane)->p1(s_loadingScreenBackgroundsByCourse[courseIndex]);
    if(!Sing_805DC5F0()) {
        if(!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl,(void*)CBpCtrl_dtor,(((LoadingStatics*)base)->bpCtrlRegistration)); s_bpCtrlInitialized=1; }
        Sing_805DC5F0()=s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(),((LoadingDrawStrings*)loadingDrawStrings)->loadingLayout,&AsCLoadingDraw(self)->bpEntryId,0);
    return self;
}
#pragma pop
struct PopStr {
    unsigned int flags;
    unsigned int size;
    void* data;
    // Release heap storage of this temporary narrow string when present.
    ~PopStr() { if (flags >> 31) appFreeUni__FPv(data); }
};

struct PopItem20 {
    char data[0x20];
    // Construct the owned item-dialog texture pack from borrowed archive descriptors.
    PopItem20(const char* name, unsigned int textureNamesAddress, int textureCount) { CPackTextureResource_Init(this, (char*)name, textureNamesAddress, (void*)textureCount); }
};

struct PopPane28 {
    char data[0x28];
    // Initialize the owned item-icon texture.
    PopPane28() { CBpTex_Init(this); }
};

struct PopVt {
    virtual void v2(int);
    virtual void v3(void*);
};

struct PopByte {
    char c __attribute__((aligned(4)));
};

#pragma push
#pragma use_lmw_stmw on
// Build the item icon filename and dialog layout, then request its message file.
void CPopItemDialog_LoadItemIconAndLayout(void* self) {
    char* base = s_bpCtrlDtorRegistration_PopItem;
    int* sw = (int*)self;
    PopStr text;
    char* dst;
    // Phase 1: load the archive and form icon basename + .gsr.
    {
        PopByte b0, b1, b2, b3;
        unsigned int textureNamesAddress = 0;
        PopStr scratch;

        if (!Sing_805DC620()) {
            if (!s_resourceTableInitialized) {
                s_resourceTableStorage = 0;
                __register_global_object(&s_resourceTableStorage, (void*)&Allocation_Dtor, (((PopItemRegistrations*)base)->resourceTable));
                s_resourceTableInitialized = 1;
            }
            Sing_805DC620() = &s_resourceTableStorage;
        }

        int textureCount = ResourceTable_FindEntryByName(Sing_805DC620(), ((PopItemDialogStrings*)popItemDialogStrings)->textureArchive, &textureNamesAddress);
        char* ptr = (char*)new PopItem20(((PopItemDialogStrings*)popItemDialogStrings)->textureArchive, textureNamesAddress, textureCount);
        AsCPopItemDialog(self)->texturePack = ptr;
        unsigned int kind = (unsigned int)sw[1] >> kItemCategoryShift;
        (*(char*)&AsCPopItemDialog(self)->isItemSet) = (kind == kItemSetCategory);

        ptr = ((IffItemCommon*)ItemInventory_FindDataWithHairStyleFallback(AsCSharedDoc(g_sharedDoc)->itemInventory, (unsigned int)sw[1]))->icon;
        text.flags = 0;
        text.size = 0;
        text.data = 0;
        unsigned int len = (unsigned int)strlen(ptr);
        String_ReserveCapacity(&text, len);
        b1.c = b0.c;
        String_ReplaceRange(&text, 0, 0, ptr, ptr + len, &b1.c);

        char* lit = ((PopItemDialogStrings*)popItemDialogStrings)->textureSuffix;
        unsigned int iconNameLength;
        if ((text.flags >> 31) == 0) iconNameLength = *(unsigned char*)&text.flags & 0x7f;
        else iconNameLength = text.size;
        b3.c = b2.c;
        String_ReplaceRange(&text, iconNameLength, 0, lit, lit + strlen(lit), &b3.c);

        void* old = AsCPopItemDialog(self)->texturePane;
        if (old) {
            if (old) ((PopVt*)old)->v2(1);
            AsCPopItemDialog(self)->texturePane = 0;
        }
        PopPane28* pane = new PopPane28();
        AsCPopItemDialog(self)->texturePane = pane;
        void* chars;
        if ((text.flags >> 31) == 0) chars = (char*)&text + 1;
        else chars = text.data;
        ((PopVt*)pane)->v3(chars);

        // Phase 2: replace the script entry with the item/set layout.
        if (sw[2] != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)base);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), sw[2]);
            sw[2] = -1;
        }

        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)base);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* sheet = Sing_805DC5F0();
        char* panelName = ((PopItemDialogStrings*)popItemDialogStrings)->itemLayout;
        if (AsCPopItemDialog(self)->isItemSet) panelName = ((PopItemDialogStrings*)popItemDialogStrings)->setLayout;
        CBpCtrl_CreateEntry(sheet, panelName, &AsCPopItemDialog(self)->bpEntryId, 0); // original +0x8

        // Phase 3: release the old message file; request the new file after strings unwind.
        void* old24 = AsCPopItemDialog(self)->sharedFile;
        if (old24) {
            CSharedFileHandle_Destroy(old24, 1);
            AsCPopItemDialog(self)->sharedFile = 0;
        }

        scratch.flags = 0;
        scratch.size = 0;
        scratch.data = 0;
        NString_AssignCString(&scratch, ((PopItemDialogStrings*)popItemDialogStrings)->messageFile);
        char* out = (((PopItemRegistrations*)base)->messageFileName);
        char* src;
        if ((scratch.flags >> 31) == 0) src = (char*)&scratch + 1;
        else src = (char*)scratch.data;
        strcpy(out, src);
        dst = (((PopItemRegistrations*)base)->messageFileName);
    }
    AsCPopItemDialog(self)->sharedFile = (void*)(long)SharedFile_Create(dst, 0);
}
#pragma pop
struct V5WStr {
    unsigned int flags;
    unsigned int size;
    void* data;
    // Release heap storage of this temporary wide string when present.
    ~V5WStr() { if (flags >> 31) appFreeUni__FPv(data); }
};
// Empty a wide string in place. Bit 31 of flags selects heap storage; otherwise the inline
// length lives in the low 7 bits of the first byte and the text starts at byte offset 2.
static inline void V5WStr_Clear(V5WStr& str) {
    if (!(str.flags >> 31)) {
        *(unsigned short*)((char*)&str + 2) = 0;
        *(unsigned char*)&str.flags &= ~0x7f;
    } else {
        *(unsigned short*)str.data = 0;
        str.size = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Lay out the item/set names and expand the reward/release description for the item category.
void CPopItemDialog_LayoutItemText(void* self) {
    extern void CMsgDraw_ResetTextState(void*);
    extern void* IdMap_Lookup(void*, void*);
    extern void* BpObject_GetOrCreateNamedGroup(void*, const char*);
    extern void UIGroup_GetPosition(void*, void*);
    extern void UIGroup_GetSourceSize(void*, void*);
    extern unsigned int UIGroup_GetTintColor(void*);
    extern void Text_SetPosition(void*, void*);
    extern void CMsgDraw_SetLayoutSize(void*, void*);
    extern void CMsgDraw_SetColor(void*, unsigned int);
    extern void* ItemInventory_FindDataWithHairStyleFallback(void*, void*);
    extern wchar_t* MessageTable_FindTextByName(void*, void*);
    extern void WString_ReplaceRange(void*, int, unsigned int, wchar_t*, wchar_t*, unsigned char*);
    extern void Text_SetContent(void*, void*);
    extern void CMsgDraw_ClearFormattingAndRevealAll(void*);
    extern void* ItemInventory_FindSetItemData(void*, void*);
    extern int sprintf(char*, const char*, ...);
    extern void Text_ExpandTaggedText(void*, void*, const char*, int);
    extern char itemRewardMessageStrings[];

    PopByte b0, b1, b2, b3;
    int layoutSize[2];
    int layoutPosition[2];
    V5WStr text;
    char memberGroupName[64];
    char* memberCursor;
    void* sheet;
    int i;
    char* setRecord;

    // Phase 1: take position/size/tint from the heading group, then submit its name.
    CMsgDraw_ResetTextState(AsCPopItemDialog(self)->textRenderer);

    BpCtrl_EnsureInstance();

    sheet = IdMap_Lookup(Sing_805DC5F0(), AsCPopItemDialog(self)->bpEntryId);
    char* name = ((PopItemDialogStrings*)popItemDialogStrings)->itemNameGroup;
    if (AsCPopItemDialog(self)->isItemSet) name = ((PopItemDialogStrings*)popItemDialogStrings)->setNameGroup;
    void* pane = BpObject_GetOrCreateNamedGroup(sheet, name);

    layoutPosition[0] = 0; layoutPosition[1] = 0;
    layoutSize[0] = 0; layoutSize[1] = 0;
    text.flags = 0; text.size = 0; text.data = 0;

    UIGroup_GetPosition(pane, layoutPosition);
    UIGroup_GetSourceSize(pane, layoutSize);

    unsigned int color = UIGroup_GetTintColor(pane);
    color = GroupTintToTextColor(color);
    Text_SetPosition(AsCPopItemDialog(self)->textRenderer, layoutPosition);
    CMsgDraw_SetLayoutSize(AsCPopItemDialog(self)->textRenderer, layoutSize);
    CMsgDraw_SetColor(AsCPopItemDialog(self)->textRenderer, color);

    wchar_t* headingText = MessageTable_FindTextByName(AsCPopItemDialog(self)->taggedText,
                               ItemInventory_FindDataWithHairStyleFallback(AsCSharedDoc(g_sharedDoc)->itemInventory, (*(void**)&AsCPopItemDialog(self)->itemId)));
    unsigned int currentTextLength;
    if (!(text.flags >> 31)) currentTextLength = *(unsigned char*)&text.flags & 0x7f;
    else currentTextLength = text.size;
    b1.c = b0.c;
    WString_ReplaceRange(&text, 0, currentTextLength, headingText, headingText + wcslen(headingText), (unsigned char*)&b1.c);
    void* chars;
    if (!(text.flags >> 31)) chars = (char*)&text + 2;
    else chars = text.data;
    Text_SetContent(AsCPopItemDialog(self)->textRenderer, chars);
    V5WStr_Clear(text);

    // Phase 2: submit each set member at its numbered group, reusing the text buffer.
    if (AsCPopItemDialog(self)->isItemSet) {
        setRecord = (char*)ItemInventory_FindSetItemData(AsCSharedDoc(g_sharedDoc)->itemInventory, (*(void**)&AsCPopItemDialog(self)->itemId));
        memberCursor = setRecord;
        i = 0;
        while (i < ((IFF_STRUCT::sSetItem*)setRecord)->itemCount) {
            sprintf(memberGroupName, ((PopItemDialogStrings*)popItemDialogStrings)->setItemGroupFormat, i + 1);
            void* item = BpObject_GetOrCreateNamedGroup(sheet, memberGroupName);
            UIGroup_GetPosition(item, layoutPosition);
            UIGroup_GetSourceSize(item, layoutSize);
            unsigned int memberColor = UIGroup_GetTintColor(item);
            memberColor = GroupTintToTextColor(memberColor);
            Text_SetPosition(AsCPopItemDialog(self)->textRenderer, layoutPosition);
            CMsgDraw_SetLayoutSize(AsCPopItemDialog(self)->textRenderer, layoutSize);
            CMsgDraw_SetColor(AsCPopItemDialog(self)->textRenderer, memberColor);

            wchar_t* memberText = MessageTable_FindTextByName(AsCPopItemDialog(self)->taggedText,
                                        ItemInventory_FindDataWithHairStyleFallback(AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)((IFF_STRUCT::sSetItem*)memberCursor)->itemIds[0]));
            unsigned int currentMemberTextLength;
            if (!(text.flags >> 31)) currentMemberTextLength = *(unsigned char*)&text.flags & 0x7f;
            else currentMemberTextLength = text.size;
            b3.c = b2.c;
            WString_ReplaceRange(&text, 0, currentMemberTextLength, memberText, memberText + wcslen(memberText), (unsigned char*)&b3.c);
            void* memberChars;
            if (!(text.flags >> 31)) memberChars = (char*)&text + 2;
            else memberChars = text.data;
            Text_SetContent(AsCPopItemDialog(self)->textRenderer, memberChars);
            V5WStr_Clear(text);
            memberCursor += 4;
            i++;
        }
    }

    // Phase 3: select a description template using mode and the upper item-ID bits.
    pane = BpObject_GetOrCreateNamedGroup(sheet, ((PopItemDialogStrings*)popItemDialogStrings)->detailGroup);
    UIGroup_GetPosition(pane, layoutPosition);
    UIGroup_GetSourceSize(pane, layoutSize);
    color = UIGroup_GetTintColor(pane);
    color = GroupTintToTextColor(color);
    Text_SetPosition(AsCPopItemDialog(self)->textRenderer, layoutPosition);
    CMsgDraw_SetLayoutSize(AsCPopItemDialog(self)->textRenderer, layoutSize);
    CMsgDraw_SetColor(AsCPopItemDialog(self)->textRenderer, color);

    if (AsCPopItemDialog(self)->descriptionMode == kDescriptionAcquisition) {
        unsigned int id = AsCPopItemDialog(self)->itemId >> kItemCategoryShift;
        switch (id) {
            case kCourseCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getCourse, 0); break;
            case kCharacterCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getCharacter, 0); break;
            case kClubSetCategory:
            case kBallCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getItem, 0); break;
            case kItemSetCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getCostumeSet, 0); break;
            case kCostumeCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getCostume, 0); break;
            default: break;
        }
    } else if (AsCPopItemDialog(self)->descriptionMode == kDescriptionCompletion) {
        int id = AsCPopItemDialog(self)->itemId >> kItemCategoryShift;
        switch (id) {
            case kBallCategory:
            case kAuxPartCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->getCompleteItem, 0); break;
            default: break;
        case 0: break;
}
    } else if (AsCPopItemDialog(self)->descriptionMode == kDescriptionRelease) {
        unsigned int id = AsCPopItemDialog(self)->itemId >> kItemCategoryShift;
        switch (id) {
            case kCaddieCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->releaseCaddie, 0); break;
            case kItemSetCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->releaseCostumeSet, 0); break;
            case kCostumeCategory:
            case kClubSetCategory:
            case kBallCategory:
            case kCaddieItemCategory: Text_ExpandTaggedText(AsCPopItemDialog(self)->taggedText, &text, ((ItemRewardMessageStrings*)itemRewardMessageStrings)->releaseCostume, 0); break;
            default: break;
        }
    }

    void* tail;
    if (!(text.flags >> 31)) tail = (char*)&text + 2;
    else tail = text.data;
    Text_SetContent(AsCPopItemDialog(self)->textRenderer, tail);
    V5WStr_Clear(text);

    CMsgDraw_ClearFormattingAndRevealAll(AsCPopItemDialog(self)->textRenderer);
}
#pragma pop
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623FC", _ctors_u802861B0_0, FileIoAccessCheck_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU804623FC", _ctors_u802861B0_1, PopItemDialog_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU804623FC", _ctors_u802861B0_2, LoadingDraw_InitTwoPiCache);
#pragma pop
