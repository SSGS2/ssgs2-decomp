#include <decomp/static_init.h>
// Top Menu submenus, Multiplayer menu and title resource/render-context setup.
// Widescreen mod lookup:
// - CTopSubMenu_Draw draws BP -> layout -> hover message -> optional modals.
//   Layout_DrawWithOpacity uses kTopSubMenuLayoutFullOpacityStorage=1.0 (first
//   binary32 word of an eight-byte slot, u802BC3F8_data.cpp). This is opacity,
//   not aspect. BP/layout/message draw calls are separate coordinate consumers.
// - Submenu message anchor: vfunc_3 reads element menuItemCount*4 in the layout.
//   Item i uses four elements: 4*i+1 is the activation target; +2/+3 hold states.
//   Change layout/script positions together with hit tests and hover text.
// - Multiplayer background: vfunc_4 takes position and source dimensions from
//   the named BP group (multiPlayTopMenuStrings+kMultiplayerBackgroundGroup).
//   Both quads use that UI rectangle; no 640/480/resolution is baked into it.
// - There is no projection, viewport or scissor setter here. Widescreen behavior
//   is inherited from BP/layout/message renderers and the active draw context.
// - Frame rate: kPlayerInfoFadeOutFrames=15 is passed to FadeState_StartFadeOut;
//   BP script completion and MenuMessage_UpdateReveal advance in the UI sequence.
// Preserve draw order and context boundaries, packed color conversion, signed
// alpha arithmetic, script result publication and secondary-interface offsets.
// Required receivers are live constructed objects except nullable deleting dtors;
// input/update methods run on the normal UI thread with valid shared singletons.
// The title owns activeMenu/textureResource. Submenu modals are created and deleted
// by vfunc_5; layout/BP/base resources follow CTopSubMenu_dtor's separate cleanup.
// Shared schemas are in classes/CTopSubMenu.h, CMultiPlayTopMenu.h and
// CPangya2TitleMain.h; u802BC3F8_data.cpp contains quad templates/scalars.
#include "classes/CSharedDoc.h"
// Borrow the existing CSharedDoc ABI view without allocation or ownership changes.
static inline CSharedDoc* AsCSharedDoc(const void* p) { return (CSharedDoc*)p; }
#include "game/vector.h"
// Translation unit 0x802BC3F8-0x802BD994 (26 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CPangya2TitleMain.h"
#include "classes/CTopSubMenu.h"
#include "classes/CMultiPlayTopMenu.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CEnemyRenderRecord.h"

// Borrow the existing CMultiPlayTopMenu ABI view without allocation or ownership changes.
static inline CMultiPlayTopMenu* AsCMultiPlayTopMenu(const void* p) { return (CMultiPlayTopMenu*)p; }

// Borrow the existing CTopSubMenu ABI view without allocation or ownership changes.
static inline CTopSubMenu* AsCTopSubMenu(const void* p) { return (CTopSubMenu*)p; }

// Borrow the existing CPangya2TitleMain ABI view without allocation or ownership changes.
static inline CPangya2TitleMain* AsCPangya2TitleMain(const void* p) { return (CPangya2TitleMain*)p; }

enum TopMenuLayout {
    kElementsPerMenuItem = 4, kPlayerInfoEntry = 13,
    kMultiplayerItemCount = 5, kMultiplayerMessageElement = 20,
    kSubMenuExtraGroup = 16, kMultiplayerScriptIndex = 9, kSubMenuScriptIndex = 10,
    kPlayerInfoFadeOutFrames = 15
};
enum TopMenuStringOffsets {
    kMultiplayerBackgroundGroup = 0x33, kMultiplayerEnterScript = 0x38,
    kMultiplayerMessageGroup = 0x3B, kMultiplayerExitScript = 0x46,
    kSubMenuMessageGroup = 3, kSubMenuExitScript = 0x0E
};
// Keep secondary-interface byte addition: taking the member address changes CodeWarrior scheduling.
enum MenuInterfaceOffset { kMenuInterfaceOffset = (int)&((CTopSubMenu*)0)->interfaceVtable };
enum TitleTaskVtable { kTitleTaskInterfaceVtableOffset = 0x3C };
enum TopMenuDrawing { kQuadVertexCount = 4, kTriangleStrip = 0x98, kMenuVertexFormat = 0x11F005 };
// UI/table contract: item i owns elements 4*i..4*i+3; 4*i+1 is hit-tested,
// and +2/+3 receive paired value-state updates. The message follows all items.
// menuMode selects a shared help/result table window starting at 0,3,8,12.
// Slot 13 opens player info; other slots publish topSubMenuResults[slot].
// mode | base | itemCount | bucket -> slot/result
//   0  |   0  |    2      | 0->0/1, 1->1/2; fallback help 2
//   1  |   3  |    4      | 0..3->3..6/all 11; fallback help 7
//   2  |   8  |    3      | 0->8/8, 1->9/9, 2->10/10; fallback help 11
//   3  |  12  |    3      | 0->12/14, 1->13/player-info, 2->14/13; fallback 15
// Back results occupy each window's final slot (2/7/11/15) and are -1. A direct
// activation of that bucket publishes cancellation; hover fallback uses that
// slot's help. Require 0<=bucket<=itemCount and the matching per-mode count;
// ordinary item hits use 0<=bucket<itemCount. No bounds/mode validation here.
// Semantic results are consumed by CPangya2TitleMain_OnProcess (title_main_and_top_menu.cpp):
// 1/2 -> CSingleTaskRev variants 0/1; 8/9 -> practice/default vs alternate;
// 10 -> CTrainingTask; 11 -> CShop2Task; 13 -> CRoundDolTask (gallery);
// 14 -> CBgmCollectionTask; -1 -> reopen parent menu. Slot 13 is intercepted
// before reading its stored 11 and opens CPlayerInfo without publishing a result.
// CPangya2TopMenu_UpdateMenuState currently constructs modes 0/2/3 with counts 2/3/3;
// mode 1 has a four-item table window but its caller's layoutIndex is -1, so it
// is NOT supported by that factory. Direct callers need their own valid layout.
enum SubMenuTableBase { kSubMenuMode1Base = 3, kSubMenuMode2Base = 8, kSubMenuMode3Base = 12 };
enum MultiplayerChoiceElement { kMultiplayerChoice1 = 1, kMultiplayerChoice2 = 5,
    kMultiplayerChoice3 = 9, kMultiplayerChoice4 = 13, kMultiplayerChoice5 = 17 };
// Result codes are externally consumed: these names identify ordinal table slots.
enum MultiplayerChoiceResult { kMultiplayerResult1 = 3, kMultiplayerResult2 = 4,
    kMultiplayerResult3 = 5, kMultiplayerResult4 = 6, kMultiplayerResult5 = 7 };
struct TopMenuVertex {
    float x, y, z;
    unsigned char red, green, blue, alpha;
    float u, v;
};
typedef char TopMenuVertex_size[(sizeof(TopMenuVertex) == 0x18) ? 1 : -1];
union TopMenuQuad { unsigned int words[24]; TopMenuVertex vertices[4]; };

// Existing RenderContext supplies vector and enemy-record storage; the save-slot
// prefix comprises four complete 0x828-byte player records.
enum TitleRenderStorage { kTitlePlayerCount = 4, kEnemyRecordCount = 4 };

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u802BC3F8 {
    char* base;
    bool active;
    // Arm only AFTER OnInit has constructed all four vectors and four enemy
    // records. This guard covers ResetPlayerRecords, not those constructors.
    explicit RenderContextConstructionCleanup_u802BC3F8(void* object)
        : base((char*)object), active(true) {}
    // Release fully constructed enemy records and vector buffers on reset unwind.
    // ResetPlayerRecords (party_main_and_versus_main.cpp) clears size, grows via a temporary vector
    // and swaps only after allocation, then copies a SavePlayerRecord per slot.
    // An allocation/save-singleton exception can leave earlier slots reset and
    // later ones unchanged, but all vector headers still own valid allocations.
    // No enemy-record construction occurs during reset; players+0x290 was set
    // to zero before entry, so CharacterPreviewTable_dtor sees a valid table.
    // Null-return allocation is unchecked in reset; this guard is an unwind
    // contract, not recovery from a failed allocation followed by invalid access.
    ~RenderContextConstructionCleanup_u802BC3F8() {
        if (active) {
            CharacterPreviewTable_dtor(((RenderContext*)base)->players, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)&((RenderContext*)base)->vectors[index];
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset already constructed context members; on exception destroy the enemy
// table and four vectors. Global destructor registration/initialized publication
// occurs only after return. Earlier construction failures are outside this guard.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u802BC3F8 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern int CTopMenuBase_PollLoad(void*);
extern int CTopMenuBase_GetFirstActivatedElement(void*);
extern int CTopMenuBase_ResetItemPlayerStates(void*);
extern void CTopMenuBase_HideItemGroup(void*, int);
extern void CTopMenuBase_SyncItemPosition(void*, int);
extern void MenuMessage_ResetTextState(void*);
extern int CTopMenuBase_HitTestPlayer(void*, int);
extern void MenuMessage_SetText(void*, void*);
extern void CTopMenuBase_ReleaseTextureResource(void*);
extern void MenuMessage_SetPosition(void*, void*);
extern void Layout_GetElementPosition(void*, int, void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void* BpObject_GetOrCreateNamedGroup(void*, const char*);
extern unsigned int UIGroup_GetTintColor(void*);
extern void MenuMessage_SetColorMultiplier(void*, void*);
extern void Layout_SetElementEnabled(void*, int, int);
extern void Layout_SetElementValueState(void*, int, int);
extern int HomeMenu_OverrideBlockedState(void*, int);
extern void HomeButtonMenu_InitializeState(const char*);
extern void HomeButtonResources_dtor(void*);
extern void IFadeMenuBase_StartMenuFadeOut(void*);
extern void CTopMenuBase_EnablePreviousElement(void*, int);
extern int Layout_PollLoadComplete(void*);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*, int);
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern void BpObject_SetEnabled(void*, int);
extern void BpObject_StartScriptByName(void*, const char*, int);
extern void CBpCtrlHandle_SetScriptIndex(void*, int);
extern void __dt__7MPBase2Fv(void*, int);
extern void CBpCtrlHandle_Dtor(void*, int);
extern void __dl__FPv(void*);
extern void __register_global_object(void*, void*, void*);
extern int CMultiPlayTopMenu_HandleSelection(char*);
extern void* CMultiPlayTopMenu_dtor(void*, int);
extern int CTopSubMenu_HandleSelection(char*);
extern void* CTopSubMenu_dtor(void*, int);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void BpObject_Draw(void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void MenuMessage_DrawText(void*);
extern void MPBase1_DrawIfInitialized(void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* UIGroup_GetPosition(void*, float*);
extern void* UIGroup_GetSourceSize(void*, float*);
extern int UIGroup_GetCombinedOpacity(void*);
extern void* GX_GetPrimitiveCount(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void DrawPrimitiveUP(int, void*, void*, int);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern unsigned int multiplayerBackgroundQuadTemplate[24];
extern unsigned int multiplayerOverlayQuadTemplate[24];
extern float kMultiplayerLayoutFullOpacityStorage;
extern void MPBase1_Ctor(void*, int, int, int, int);
extern void CTopMenuBase_InitMembers(void*, int, int, int, const char*);
extern void __dt__16CWarningLogoBaseFv(void*);
extern void MenuMessage_DrawText(void*);
extern void Layout_Update(void*);
extern void* ResourceTable_FindEntryByName(void*, const char*, int*);
extern void CPackTextureResource_Init(void*, const char*, int, void*);
extern void CPackTextureResource_PollLoad(void*);
extern int CPackTextureResource_IsLoadFinished(void*);
extern void Allocation_Dtor(void*);
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern char modeTransitionStrings[];
extern char s_resourceTableDtorRegistration_TitleMain[];
extern signed char s_renderContextInitialized;
extern char s_renderContext[];
extern char s_renderContextDtorRegistration_TitleMain[];
extern void SaveData_InitPlayerRecord(void*);


extern void __construct_array(void*, void*, void*, int, int);
extern void PartyItemRecord_InitDefaults(void*);
extern void PlayerRenderRecord_dtor(void*);
extern void __dt__13RenderContextFv(void*);
extern void RenderContext_ResetPlayerRecords(void*);
extern CSharedDoc* g_sharedDoc;
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void*);
extern void MovieController_ReleaseAudioAndResetLatches(void*);
extern char s_movieController[];
extern char s_movieControllerDtorRegistration_TitleMain[];
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void* __nw__FUl(unsigned int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void COptionDialog_Ctor(void*, int);
extern int MPBase1_Process(void*, float);
extern void SaveData_Ctor(void*);
extern void* SaveData_GetCurrentRecord(void*);
extern void CPlayerInfo_Ctor(void*, void*);
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void);
extern void FadeState_RequestFramebufferCapture(void*);
extern void FadeState_StartFadeIn(void*, int);
extern void FadeState_StartFadeOut(void*, int);
extern void CTopMenuBase_UpdateHoverElementPairs(void*);
extern void CTopMenuBase_ProcessInput(void*);
extern void MenuMessage_UpdateReveal(void*);
extern int CTopMenuBase_HitTestPlayer(void*, int);
extern signed char s_saveDataInitialized;
extern signed char s_fadeOverlayInitialized;
extern char s_saveData[];
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_TopSubMenu[];
extern signed char s_movieControllerInitialized;
extern float kFloat_6_2831855;
extern float kTopSubMenuLayoutFullOpacityStorage;
extern float s_multiplayerTopMenuTwoPiStorage;
extern float s_topSubMenuTwoPiStorage;
extern char __vt__17CPangya2TitleMain[];
extern char topSubMenuElementIds[];
extern char __vt__11CTopSubMenu[];
extern char s_bpCtrl[];
extern char multiPlayTopMenuStrings[];
extern char topSubMenuScriptStrings[];
extern int topSubMenuResults[];
extern void* topSubMenuHelpMessages[];
extern signed char s_homeMenuInitialized;
extern char s_homeButtonResourceState[];
extern char s_multiPlayHomeMenuDtorRegistration[];
extern char s_multiPlayBpHandleDtorRegistration[];
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlDtorRegistration_TopSubMenu[];
extern char s_bpCtrlHandleDtorRegistration_TopSubMenu[];
extern char s_bpCtrlDtorRegistration_TopSubMenu[];
struct VTable { void* a; void* b; void (*destroy)(void*, int); };
struct DynObj { 
virtual void destroy(int);
};

struct Alloc860 {
    char pad[0x860];
    // Construct the owned options dialog using the original dialog selector.
    Alloc860(int x) { COptionDialog_Ctor(this, x); }
};
struct AllocA8 {
    char pad[0xA8];
    // Construct the owned player-info dialog over the borrowed save record.
    AllocA8(void* x) { CPlayerInfo_Ctor(this, x); }
};
struct AllocTask {
    char pad[0x20];
    // Construct the owned texture-pack loader from its name, size and resource.
    AllocTask(const char* s, int a, void* r) { CPackTextureResource_Init(this, s, a, r); }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Advance menu scripts or modal dialogs; otherwise update hover text and process submenu input.
int CTopSubMenu_UpdateScriptsDialogsAndInput(void* self, float dt) {
    // Priority order is exclusive: script -> completed selection -> options ->
    // player info -> ordinary input. Return 1 only after selection's script ends;
    // dt is nonnegative simulated SECONDS per update, forwarded unchanged through
    // MPBase1_Process (event_face_and_mpbase1_menu.cpp). The main-frame seconds hook is produced from
    // reciprocal 25/30 updates/s in CMainFrame_UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp).
    // COptionDialog_UpdateAndProcessInput (option_dialog_control_mode_audio.cpp) subtracts dt from its inputDelay in seconds
    // and forwards it to its save menu; CPlayerInfo_UpdatePageInputAndDispatch (player_info_screen.cpp) ignores
    // the float ABI argument and uses integer scroll calls/BP completion instead.
    // No local timer multiplies dt here; 15-call overlay fades remain call-based.
    int result = 0;
    if (AsCTopSubMenu(self)->scriptRunning) {
        // Phase 1: scripts own positioning and block new input until completion.
        for (int i = 0; i < AsCTopSubMenu(self)->menuItemCount * kElementsPerMenuItem; ++i)
            CTopMenuBase_SyncItemPosition(&AsCTopSubMenu(self)->interfaceVtable, i); // original +0x18
        int _r = (BpObject_IsCurrentScriptComplete(AsCTopSubMenu(self)->bpObject) == 0);
        AsCTopSubMenu(self)->scriptRunning = (unsigned char)_r;
    } else if (AsCTopSubMenu(self)->selectionComplete) {
        result = 1;
    } else if (AsCTopSubMenu(self)->optionDialogRequested) {
        // Phase 2a: lazily create, process and delete the owned options modal.
        if (!AsCTopSubMenu(self)->optionDialog) {
            AsCTopSubMenu(self)->optionDialog = new Alloc860(1);
        }
        if (MPBase1_Process(AsCTopSubMenu(self)->optionDialog, dt)) {
            void* q = AsCTopSubMenu(self)->optionDialog;
            if (q) {
                if (q) ((DynObj*)q)->destroy(1);
            }
            AsCTopSubMenu(self)->optionDialog = 0;
            AsCTopSubMenu(self)->optionDialogRequested = 0;
        }
    } else if (AsCTopSubMenu(self)->playerInfoRequested) {
        // Phase 2b: create player info against the current save record and fade in.
        // Singleton creation below registers module cleanup, not dialog ownership.
        if (!AsCTopSubMenu(self)->playerInfo) {
            AsCTopSubMenu(self)->playerInfo = new AllocA8(SaveData_GetCurrentRecord(
                Sing_805DC5E0() ? Sing_805DC5E0() :
                (s_saveDataInitialized ? 0 : (SaveData_Ctor(s_saveData), s_saveDataInitialized = 1, 0),
                 Sing_805DC5E0() = s_saveData,
                 Sing_805DC5E0())
            ));
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_TopSubMenu);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_TopSubMenu);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_StartFadeIn(Sing_805DC608(), 0);
        }
        if (MPBase1_Process(AsCTopSubMenu(self)->playerInfo, dt)) {
            // Modal completion deletes it, re-enters the menu script and fades out.
            void* q = AsCTopSubMenu(self)->playerInfo;
            if (q) {
                if (q) ((DynObj*)q)->destroy(1);
            }
            AsCTopSubMenu(self)->playerInfo = 0;
            AsCTopSubMenu(self)->playerInfoRequested = 0;
            AsCTopSubMenu(self)->scriptRunning = 1;
            BpObject_StartScriptByName(AsCTopSubMenu(self)->bpObject, topSubMenuScriptStrings, 1);
            if (!Sing_805DC608()) {
                if (!s_fadeOverlayInitialized) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_TopSubMenu);
                    s_fadeOverlayInitialized = 1;
                }
                Sing_805DC608() = s_fadeOverlay;
            }
            FadeState_StartFadeOut(Sing_805DC608(), kPlayerInfoFadeOutFrames);
        }
    } else {
        // Phase 3: resolve hovered help text (fallback is one slot after the last
        // visible item), then process hover pairs and selection input.
        Layout_Update(AsCTopSubMenu(self)->layout);
        MenuMessage_ResetTextState(AsCTopSubMenu(self)->menuMessage);
        int found = 0;
        for (int i = 0, index = 1; i < AsCTopSubMenu(self)->menuItemCount; ++i, index += kElementsPerMenuItem) {
            if (CTopMenuBase_HitTestPlayer(&AsCTopSubMenu(self)->interfaceVtable, index)) { // original +0x18
                int bucket = (i * kElementsPerMenuItem + 1) / kElementsPerMenuItem;
                int mode = AsCTopSubMenu(self)->menuMode;
                int slot = -1;
                switch (mode) {
                case 0: slot = bucket; break;
                case 1: slot = bucket + kSubMenuMode1Base; break;
                case 2: slot = bucket + kSubMenuMode2Base; break;
                case 3: slot = bucket + kSubMenuMode3Base; break;
                }
                MenuMessage_SetText(AsCTopSubMenu(self)->menuMessage, topSubMenuHelpMessages[slot]);
                found = 1;
                break;
            }
        }
        if (!found) {
            int bucket = ((AsCTopSubMenu(self)->menuItemCount - 1) * kElementsPerMenuItem + 1) / kElementsPerMenuItem;
            int mode = AsCTopSubMenu(self)->menuMode;
            int slot = -1;
            switch (mode) {
            case 0: slot = bucket; break;
            case 1: slot = bucket + kSubMenuMode1Base; break;
            case 2: slot = bucket + kSubMenuMode2Base; break;
            case 3: slot = bucket + kSubMenuMode3Base; break;
            }
            MenuMessage_SetText(AsCTopSubMenu(self)->menuMessage, *(void**)((char*)topSubMenuHelpMessages + slot * sizeof(void*) + sizeof(void*)));
        }
        CTopMenuBase_UpdateHoverElementPairs(&AsCTopSubMenu(self)->interfaceVtable); // original +0x18
        CTopMenuBase_ProcessInput(&AsCTopSubMenu(self)->interfaceVtable); // original +0x18
    }
    // Every branch still services BP callbacks and message reveal once per update.
    BpObject_TryInvokeCurrentCallback(AsCTopSubMenu(self)->bpObject, 0);
    MenuMessage_UpdateReveal(AsCTopSubMenu(self)->menuMessage);
    return result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Enter a submenu, synchronize its four-element item groups and select its BP script.
// menuMode must be 0..3, menuItemCount positive, and help/result windows large
// enough for that mode plus fallback. No bounds checks: unsupported mode uses -1.
// Saves the old shared BP script index, sets the submenu script, and resets message
// reveal. Borrowed group/layout positions are copied before input processing.
// Keep repeated singleton lookups: initialization registers shared cleanup, not
// ownership of this menu; changing their order can change exception side effects.
void CTopSubMenu_Show(void* self) {
    int i;
    int selected;
    int offset;
    if (AsCTopSubMenu(self)->layout) {
        i = 0;
        offset = 1;
        for (; i < AsCTopSubMenu(self)->menuItemCount; offset += kElementsPerMenuItem, ++i)
            Layout_SetElementEnabled(AsCTopSubMenu(self)->layout, offset, 0);
    }
    BpObject_SetEnabled(AsCTopSubMenu(self)->bpObject, 1);
    BpObject_StartScriptByName(AsCTopSubMenu(self)->bpObject, topSubMenuScriptStrings, 1);
    AsCTopSubMenu(self)->scriptRunning = 1;
    int j = 0;
    for (; j < AsCTopSubMenu(self)->menuItemCount * kElementsPerMenuItem; ++j) {
        CTopMenuBase_HideItemGroup(&AsCTopSubMenu(self)->interfaceVtable, j); // original +0x18
        CTopMenuBase_SyncItemPosition(&AsCTopSubMenu(self)->interfaceVtable, j); // original +0x18
    }
    CTopMenuBase_HideItemGroup(&AsCTopSubMenu(self)->interfaceVtable, 16); // original +0x18
    BpObject_TryInvokeCurrentCallback(AsCTopSubMenu(self)->bpObject, 0);
    int messagePositionWords[2] = {0, 0};
    Layout_GetElementPosition(AsCTopSubMenu(self)->layout, AsCTopSubMenu(self)->menuItemCount * kElementsPerMenuItem, messagePositionWords);
    MenuMessage_SetPosition(AsCTopSubMenu(self)->menuMessage, messagePositionWords);
    void* messageGroup = (void*)BpObject_GetOrCreateNamedGroup(AsCTopSubMenu(self)->bpObject, topSubMenuScriptStrings + kSubMenuMessageGroup);
    unsigned int color = UIGroup_GetTintColor(messageGroup);
    // Packed input bytes [A,R,G,B] become [B,G,R,FF] for the message multiplier:
    // reverse the RGB byte order and force opaque alpha, discarding input A.
    unsigned int redAndOpaqueAlpha = (color & 0x00ff0000u) >> 8;
    unsigned int blueAndGreen = (color << 8) & 0x00ff0000u;
    blueAndGreen |= (color & 0x000000ffu) << 24;
    redAndOpaqueAlpha |= 0xff;
    color = blueAndGreen | redAndOpaqueAlpha;
    MenuMessage_SetColorMultiplier(AsCTopSubMenu(self)->menuMessage, &color);
    Layout_SetElementEnabled(AsCTopSubMenu(self)->layout, AsCTopSubMenu(self)->menuItemCount * kElementsPerMenuItem, 0);
    MenuMessage_ResetTextState(AsCTopSubMenu(self)->menuMessage);
    selected = 0;
    i = 0;
    offset = 1;
    for (; i < AsCTopSubMenu(self)->menuItemCount; offset += kElementsPerMenuItem, ++i) {
        if (CTopMenuBase_HitTestPlayer(&AsCTopSubMenu(self)->interfaceVtable, offset)) { // original +0x18
            int bucket = (i * kElementsPerMenuItem + 1) / kElementsPerMenuItem;
            int slot = -1;
            switch (AsCTopSubMenu(self)->menuMode) {
            case 0: slot = bucket; break;
            case 1: slot = bucket + kSubMenuMode1Base; break;
            case 2: slot = bucket + kSubMenuMode2Base; break;
            case 3: slot = bucket + kSubMenuMode3Base; break;
            }
            MenuMessage_SetText(AsCTopSubMenu(self)->menuMessage, topSubMenuHelpMessages[slot]);
            selected = 1;
            break;
        }
    }
    if (!selected) {
        int bucket = ((AsCTopSubMenu(self)->menuItemCount - 1) * kElementsPerMenuItem + 1) / kElementsPerMenuItem;
        int slot = -1;
        switch (AsCTopSubMenu(self)->menuMode) {
        case 0: slot = bucket; break;
        case 1: slot = bucket + kSubMenuMode1Base; break;
        case 2: slot = bucket + kSubMenuMode2Base; break;
        case 3: slot = bucket + kSubMenuMode3Base; break;
        }
        MenuMessage_SetText(AsCTopSubMenu(self)->menuMessage, *(void**)((char*)topSubMenuHelpMessages + slot * sizeof(void*) + sizeof(void*)));
    }
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_TopSubMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    void* _g = Sing_805DC5F8();
    if (_g) { }
    (*(void**)&AsCTopSubMenu(self)->previousScriptIndex) = ((void**)_g)[1];
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_TopSubMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kSubMenuScriptIndex);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Open player info for entry 13, or publish the selected mode and play the exit script.
int CTopSubMenu_HandleSelection(char* self) {
    int index = CTopMenuBase_GetFirstActivatedElement(&AsCTopSubMenu(self)->interfaceVtable); // original +0x18
    if (index == -1) return 0;
    int bucket = index / kElementsPerMenuItem;
    int mode = AsCTopSubMenu(self)->menuMode;
    int slot = -1;
    switch (mode) {
    case 0: slot = bucket; break;
    case 1: slot = bucket + kSubMenuMode1Base; break;
    case 2: slot = bucket + kSubMenuMode2Base; break;
    case 3: slot = bucket + kSubMenuMode3Base; break;
    }
    if (slot == kPlayerInfoEntry) {
        AsCTopSubMenu(self)->playerInfoRequested = 1;
        AsCTopSubMenu(self)->scriptRunning = 1;
        BpObject_StartScriptByName(AsCTopSubMenu(self)->bpObject, topSubMenuScriptStrings + kSubMenuExitScript, 1);
        return 1;
    }
    int slot2 = -1;
    switch (mode) {
    case 0: slot2 = bucket; break;
    case 1: slot2 = bucket + kSubMenuMode1Base; break;
    case 2: slot2 = bucket + kSubMenuMode2Base; break;
    case 3: slot2 = bucket + kSubMenuMode3Base; break;
    }
    AsCTopSubMenu(self)->resultReady = 1; // original +0x28
    int val = topSubMenuResults[slot2];
    AsCTopSubMenu(self)->selectionComplete = 1;
    AsCTopSubMenu(self)->result = val;
    CTopMenuBase_EnablePreviousElement(&AsCTopSubMenu(self)->interfaceVtable, index); // original +0x18
    BpObject_StartScriptByName(AsCTopSubMenu(self)->bpObject, topSubMenuScriptStrings + kSubMenuExitScript, 1);
    AsCTopSubMenu(self)->scriptRunning = 1;
    return 1;
}
#pragma pop

// Enter the five-choice multiplayer menu and initialize its script, hover text and button states.
void CMultiPlayTopMenu_StartMenu(void* self) {
    if (!Sing_805DC820()) {
        if (!s_homeMenuInitialized) {
            HomeButtonMenu_InitializeState(s_homeButtonResourceState);
            __register_global_object(s_homeButtonResourceState, (void*)HomeButtonResources_dtor, s_multiPlayHomeMenuDtorRegistration);
            s_homeMenuInitialized = 1;
        }
        Sing_805DC820() = s_homeButtonResourceState;
    }
    HomeMenu_OverrideBlockedState(Sing_805DC820(), 0);
    CTopMenuBase_ResetItemPlayerStates(&AsCMultiPlayTopMenu(self)->secondaryVtable); // original +0x18
    BpObject_StartScriptByName(AsCMultiPlayTopMenu(self)->bpObject, multiPlayTopMenuStrings + kMultiplayerEnterScript, 1);
    AsCMultiPlayTopMenu(self)->scriptRunning = 1;
    MenuMessage_ResetTextState(AsCMultiPlayTopMenu(self)->menuMessage);
    for (int i = 0; i < kMultiplayerMessageElement; ++i) {
        CTopMenuBase_HideItemGroup(&AsCMultiPlayTopMenu(self)->secondaryVtable, i); // original +0x18
        CTopMenuBase_SyncItemPosition(&AsCMultiPlayTopMenu(self)->secondaryVtable, i); // original +0x18
    }
    BpObject_TryInvokeCurrentCallback(AsCMultiPlayTopMenu(self)->bpObject, 0);
    int pair[2] = {0, 0};
    Layout_GetElementPosition(AsCMultiPlayTopMenu(self)->layout, kMultiplayerMessageElement, pair);
    MenuMessage_SetPosition(AsCMultiPlayTopMenu(self)->menuMessage, pair);
    void* extraReceiver2 = (void*)BpObject_GetOrCreateNamedGroup(AsCMultiPlayTopMenu(self)->bpObject, multiPlayTopMenuStrings + kMultiplayerMessageGroup);
    unsigned int color = UIGroup_GetTintColor(extraReceiver2);
    // Apply the same [A,R,G,B] -> [B,G,R,FF] byte mapping as the submenu.
    unsigned int _r0 = (color >> 8) & 0x0000ff00u;
    unsigned int _r5 = ((color << 8) & 0x00ff0000u) | (color << 24);
    _r0 |= 0xff;
    color = _r5 | _r0;
    MenuMessage_SetColorMultiplier(AsCMultiPlayTopMenu(self)->menuMessage, &color);
    Layout_SetElementEnabled(AsCMultiPlayTopMenu(self)->layout, kMultiplayerMessageElement, 0);
    if (AsCMultiPlayTopMenu(self)->layout && Layout_PollLoadComplete(AsCMultiPlayTopMenu(self)->layout)) {
        int n = 0;
        int item = 2;
        do {
            Layout_SetElementValueState(AsCMultiPlayTopMenu(self)->layout, item, 1);
            Layout_SetElementValueState(AsCMultiPlayTopMenu(self)->layout, item + 1, 1);
            ++n;
            item += kElementsPerMenuItem;
        } while (n < kMultiplayerItemCount);
    }
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_multiPlayBpHandleDtorRegistration);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kMultiplayerScriptIndex);
    IFadeMenuBase_StartMenuFadeOut(self);
    CTopMenuBase_ReleaseTextureResource(&AsCMultiPlayTopMenu(self)->secondaryVtable); // original +0x18
}

// Map activated elements 1/5/9/13/17 to results 3-7, then publish and animate the selection.
int CMultiPlayTopMenu_HandleSelection(char* self) {
    int index = CTopMenuBase_GetFirstActivatedElement(self + kMenuInterfaceOffset);
    if (index == -1) return 0;
    switch ((unsigned int)index) {
    case kMultiplayerChoice1:  AsCMultiPlayTopMenu(self)->result = kMultiplayerResult1; break; // original +0x24
    case kMultiplayerChoice2:  AsCMultiPlayTopMenu(self)->result = kMultiplayerResult2; break;
    case kMultiplayerChoice3:  AsCMultiPlayTopMenu(self)->result = kMultiplayerResult3; break;
    case kMultiplayerChoice4:  AsCMultiPlayTopMenu(self)->result = kMultiplayerResult4; break;
    case kMultiplayerChoice5:  AsCMultiPlayTopMenu(self)->result = kMultiplayerResult5; break;
    case 0: case 2: case 3: case 4: case 6: case 7: case 8: case 10: case 11: case 12: case 14: case 15: case 16:       
    default: break;
    }
    AsCMultiPlayTopMenu(self)->resultReady = 1; // original +0x28
    AsCMultiPlayTopMenu(self)->exitRequested = 1;
    BpObject_StartScriptByName(AsCMultiPlayTopMenu(self)->bpObject, multiPlayTopMenuStrings + kMultiplayerExitScript, 1);
    AsCMultiPlayTopMenu(self)->scriptRunning = 1; // Wait for the exit script to finish.
    CTopMenuBase_EnablePreviousElement(self + kMenuInterfaceOffset, index);
    return 1;
}

// Disable the submenu BP object, restore the previous script and destroy its menu base.
void* CTopSubMenu_dtor(void* self, int deleting) {
    if (self) {
    char* q = (char*)self;
    *(void**)q = __vt__11CTopSubMenu;
    AsCTopSubMenu(q)->interfaceVtable = __vt__11CTopSubMenu + 0x28;
    BpObject_SetEnabled(AsCTopSubMenu(q)->bpObject, 0);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_TopSubMenu);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), AsCTopSubMenu(q)->previousScriptIndex);
    __dt__7MPBase2Fv(q + kMenuInterfaceOffset, 0);
    if (deleting > 0) __dl__FPv(q);
    }
    return self;
}

// Report ready when both the submenu layout and all BP entries have loaded.
int CTopSubMenu_PollLoadComplete(void* self) {
    if (!Layout_PollLoadComplete(AsCTopSubMenu(self)->layout)) return 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_TopSubMenu);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (!CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0())) return 0;
    return 1;
}

// Draw BP graphics, layout, hover message, then options and player-info modals.
// Requires ready base resources; null modals are skipped. No resource ownership
// moves. The opacity scalar is normalized 1.0, consumed separately by layout;
// changing it does not fade the BP object or hover text. Preserve call order for
// layered drawing and the external widescreen draw-context recognition sites.
void CTopSubMenu_Draw(void* self) {
    BpObject_Draw(AsCTopSubMenu(self)->bpObject);
    Layout_DrawWithOpacity(AsCTopSubMenu(self)->layout, kTopSubMenuLayoutFullOpacityStorage);
    MenuMessage_DrawText(AsCTopSubMenu(self)->menuMessage);
    void* q = AsCTopSubMenu(self)->optionDialog;
    if (q) MPBase1_DrawIfInitialized(q);
    q = AsCTopSubMenu(self)->playerInfo;
    if (q) MPBase1_DrawIfInitialized(q);
}

#pragma push
#pragma use_lmw_stmw on
class AlphaMenu { public: 
virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14(int);
};
// Draw the multiplayer background/half-alpha overlay, then BP, layout, text and options.
// Required named background group supplies position/dimensions in UI coordinates.
// Copies template Z/color/UVs, replacing XY and alpha only. Triangle-strip order
// is top-left, top-right, bottom-left, bottom-right, 0x18 bytes per vertex.
// Texture hook slot +0x14 receives 2*groupOpacity-1; vertex alpha uses its masked
// nonnegative low byte. The second quad halves it with signed rounding.
// Clears texture stage/resource before the untextured overlay; no general render
// state restore is performed. Do not fold these draws across context boundaries.
void CMultiPlayTopMenu_Draw(void* self) {
    float position[2] = {0.0f, 0.0f};
    float dimensions[2] = {0.0f, 0.0f};
    void* data = BpObject_GetOrCreateNamedGroup(AsCMultiPlayTopMenu(self)->bpObject, multiPlayTopMenuStrings + kMultiplayerBackgroundGroup);
    UIGroup_GetPosition(data, position);
    UIGroup_GetSourceSize(data, dimensions);
    int doubledOpacity = UIGroup_GetCombinedOpacity(data) * 2;
    // Let x=2*opacity-1. For its normal bounded input range, the sign mask yields
    // max(x,0); it does not clamp the upper bound. The texture hook receives raw x,
    // while vertex alpha takes the low byte of the nonnegative result.
    doubledOpacity = doubledOpacity - 1;
    int alpha = doubledOpacity & ((-doubledOpacity & ~doubledOpacity) >> 31);
    ((*(AlphaMenu**)&AsCMultiPlayTopMenu(self)->menuTexture))->v14(doubledOpacity);

    struct CopyBlock { unsigned int v[24]; };
    TopMenuQuad first;
    *(CopyBlock*)&first = *(CopyBlock*)multiplayerBackgroundQuadTemplate;
    first.vertices[0].x = position[0];
    first.vertices[0].y = position[1];
    first.vertices[0].alpha = (unsigned char)alpha;
    first.vertices[1].x = position[0] + dimensions[0];
    first.vertices[1].y = position[1];
    first.vertices[1].alpha = (unsigned char)alpha;
    first.vertices[2].x = position[0];
    first.vertices[2].y = position[1] + dimensions[1];
    first.vertices[2].alpha = (unsigned char)alpha;
    first.vertices[3].x = position[0] + dimensions[0];
    first.vertices[3].y = position[1] + dimensions[1];
    first.vertices[3].alpha = (unsigned char)alpha;
    void* prim = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
    Engine_SetFlexibleVertexFormat(kMenuVertexFormat);
    DrawPrimitiveUP(kTriangleStrip, prim, first.words, sizeof(TopMenuVertex));
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);

    // Untextured overlay uses the same screen-pixel rectangle, half the first
    // alpha (signed division toward zero encoded by add-sign-and-shift).
    TopMenuQuad second;
    *(CopyBlock*)&second = *(CopyBlock*)multiplayerOverlayQuadTemplate;
    int overlayAlpha = (((int)((unsigned int)alpha >> 31)) + alpha) >> 1;
    second.vertices[0].x = position[0];
    second.vertices[0].y = position[1];
    second.vertices[0].alpha = (unsigned char)overlayAlpha;
    second.vertices[1].x = position[0] + dimensions[0];
    second.vertices[1].y = position[1];
    second.vertices[1].alpha = (unsigned char)overlayAlpha;
    second.vertices[2].x = position[0];
    second.vertices[2].y = position[1] + dimensions[1];
    second.vertices[2].alpha = (unsigned char)overlayAlpha;
    second.vertices[3].x = position[0] + dimensions[0];
    second.vertices[3].y = position[1] + dimensions[1];
    second.vertices[3].alpha = (unsigned char)overlayAlpha;
    prim = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
    Engine_SetFlexibleVertexFormat(kMenuVertexFormat);
    DrawPrimitiveUP(kTriangleStrip, prim, second.words, sizeof(TopMenuVertex));
    BpObject_Draw(AsCMultiPlayTopMenu(self)->bpObject);
    Layout_DrawWithOpacity(AsCMultiPlayTopMenu(self)->layout, kMultiplayerLayoutFullOpacityStorage);
    MenuMessage_DrawText(AsCMultiPlayTopMenu(self)->menuMessage);
    void* q = AsCMultiPlayTopMenu(self)->optionDialog;
    if (q) MPBase1_DrawIfInitialized(q);
}
#pragma pop
class Y3 { public: 
virtual void v08(int);
};
// Invalidate the title's active menu before destroying its texture-pack loader.
// CPangya2TitleMain_OnProcess (title_main_and_top_menu.cpp) creates/replaces activeMenu. Menu and
// pack borrowers must stop update/draw/message
// dispatch before this hook; deleting the menu first ends those borrows, then
// deleting the pack ends its loader/resource lifetime. Neither pointer may be
// cached by a caller past teardown. Both owner slots are cleared; repeated calls
// skip them. Shared render/BP singletons outlive this hook and are not deleted.
void CPangya2TitleMain_OnDestroy(void* self) {
    if (AsCPangya2TitleMain(self)->activeMenu) {
        void* q = AsCPangya2TitleMain(self)->activeMenu;
        if (q) {
            ((Y3*)q)->v08(1);
        }
        AsCPangya2TitleMain(self)->activeMenu = 0;
    }
    if (AsCPangya2TitleMain(self)->textureResource) {
        void* q = AsCPangya2TitleMain(self)->textureResource;
        if (q) {
            ((Y3*)q)->v08(1);
        }
        AsCPangya2TitleMain(self)->textureResource = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Initialize submenu base/interfaces and clear result/player-info ownership.
// c is menuMode, with counts/bounds in the table map above. a is a borrowed loaded
// layout address encoded as int; b is its borrowed BP object address; d is the
// menu item count, NOT pixels or a BP entry ID. CTopMenuBase_InitMembers
// (top_menu_base_and_menu_message.cpp) stores a/b/d at interface+4/+8/+14 and sets borrowsResources=1;
// layout must provide the four elements per item, final message anchor and extra
// group 16 used here; BP must provide in/out and R_MSG_info groups. Caller owns
// both through submenu destruction. The base creates its own MenuMessage helper.
// MPBase1_Ctor (event_face_and_mpbase1_menu.cpp) only consumes self; its extra a/b/c/d ABI words here
// are ignored. CPangya2TopMenu_UpdateMenuState/TopSubMenu (title_main_and_top_menu.cpp) supplies a layout
// and companion BP word and the per-mode count; see its unsupported mode-1 case.
// Fresh self only; returns self. __dt__7MPBase2Fv (top_menu_base_and_menu_message.cpp) tests the stored
// borrowsResources flag: with 1 it skips layout destruction, BP-entry deletion
// and texture release. It still destroys the owned MenuMessage/options helper.
// CTopSubMenu_dtor restores the prior BP script index before that base cleanup.
// Thus the factory's layout/BP owner must outlive the submenu; this constructor
// does not transfer those resources to base cleanup.
// The secondary interface lives at +0x18 and receives vtable+0x28, not the owner
// vtable. Preserve that distinction in all callbacks and deleting thunks.
void* CTopSubMenu_Ctor(char* self, int a, int b, int c, int d) {
    MPBase1_Ctor(self, a, b, c, d);
    CTopMenuBase_InitMembers(self + kMenuInterfaceOffset, a, b, d, topSubMenuElementIds);
    *(void**)self = __vt__11CTopSubMenu;
    AsCTopSubMenu(self)->interfaceVtable = __vt__11CTopSubMenu + 0x28;
    (*(void**)&AsCTopSubMenu(self)->menuMode) = (void*)c;
    AsCTopSubMenu(self)->result = -1;
    (*(int*)&AsCTopSubMenu(self)->playerInfo) = 0;
    AsCTopSubMenu(self)->playerInfoRequested = 0;
    AsCTopSubMenu(self)->previousScriptIndex = -1;
    return (void*)self;
}
#pragma pop

// Initialize the submenu module angle constant to one full turn.
void TopSubMenu_InitFullTurnConstant(void) {
    s_topSubMenuTwoPiStorage = kFloat_6_2831855;
}

// Copy the published submenu result to required out; return 1 if available.
// Return 0 leaves *out unchanged. Reading does not consume/reset resultReady.
int CTopSubMenu_TryGetResult(void* self, int* out) {
    if (AsCTopSubMenu(self)->resultReady) { // original +0x28
        *out = AsCTopSubMenu(self)->result;
        return 1;
    }
    return 0;
}

// Initialize title-task interfaces and clear menu/resource ownership.
void* CPangya2TitleMain_ctor(void* self) {
    __ct__18NativeBase80201D18Fv(self);
    void* base = __vt__17CPangya2TitleMain;
    int zero = 0;
    *(void**)self = base;
    AsCPangya2TitleMain(self)->interfaceVtable = (char*)base + kTitleTaskInterfaceVtableOffset;
    (*(int*)&AsCPangya2TitleMain(self)->activeMenu) = zero;
    AsCPangya2TitleMain(self)->topMenuResult = -1;
    (*(int*)&AsCPangya2TitleMain(self)->textureResource) = zero;
    return self;
}

// Title pre-load hook; no setup is required.
void CPangya2TitleMain_OnPreLoadInit(void) {
}

// Own a mode-transition texture-pack loader and synchronously poll it to completion.
// Requires resource table initialization; name/metadata are borrowed, loader is
// deleted by OnDestroy/dtor. No timeout, failure branch or previous-loader release
// occurs here: invoke once per load lifecycle. Pool name is modeTransitionStrings.
void CPangya2TitleMain_OnLoad(void* self) {
    int arg = 0;
    if (!Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, s_resourceTableDtorRegistration_TitleMain);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    int result = (int)ResourceTable_FindEntryByName(Sing_805DC620(), modeTransitionStrings, &arg);
    AllocTask* task = new AllocTask(modeTransitionStrings, arg, (void*)result);
    AsCPangya2TitleMain(self)->textureResource = task;
    // PollLoad clears the pending archive path when loading finishes.
    while (!CPackTextureResource_IsLoadFinished(AsCPangya2TitleMain(self)->textureResource)) CPackTextureResource_PollLoad(AsCPangya2TitleMain(self)->textureResource);
}

// Initialize shared four-player render storage, then reset it and title menu state.
// On first use, constructs save-record/vector/enemy-record regions and registers
// global destruction. InitializeNewRenderContext guards reset AFTER complete
// vector/enemy-record construction (see its failure ledger). Every call resets player records, partyGameMode=0 and
// menuState=-1; title does not take ownership of the shared singleton.
void CPangya2TitleMain_OnInit(char* self) {
    char* end;
    char* cur;
    char* member;
    char* base;
    if (!Sing_805DC648()) {
        if (!s_renderContextInitialized) {
            base = s_renderContext;
            cur = base;
            end = (char*)&((RenderContext*)base)->vectors;
            do {
                SaveData_InitPlayerRecord(cur);
                cur += sizeof(SavePlayerRecord);
            } while (cur < end);
            ((IntVectorArray*)end)->__ct();
            member = ((RenderContext*)base)->players;
            __construct_array(member, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord), kEnemyRecordCount);
            *(int*)(member + 0x290) = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextDtorRegistration_TitleMain);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    RenderContext_ResetPlayerRecords(Sing_805DC648());
    (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) = 0;
    AsCPangya2TitleMain(self)->menuState = -1;
}

// Stop the movie controller, destroy title resources and optionally free the title task.
void* CPangya2TitleMain_dtor(void* self, int deleting) {
    if (self) {
        *(void**)self = __vt__17CPangya2TitleMain;
        AsCPangya2TitleMain(self)->interfaceVtable = __vt__17CPangya2TitleMain + 0x3c;
        if (!Sing_805DC950()) {
            if (!s_movieControllerInitialized) {
                MovieController_Ctor(s_movieController);
                __register_global_object(s_movieController, (void*)MovieController_Dtor, s_movieControllerDtorRegistration_TitleMain);
                s_movieControllerInitialized = 1;
            }
            Sing_805DC950() = s_movieController;
        }
        MovieController_ReleaseAudioAndResetLatches(Sing_805DC950());
        void* q = AsCPangya2TitleMain(self)->activeMenu;
        if (q) {
            if (q) {
((Y3*)q)->v08(1);
            }
            AsCPangya2TitleMain(self)->activeMenu = 0;
        }
        q = AsCPangya2TitleMain(self)->textureResource;
        if (q) {
            if (q) {
((Y3*)q)->v08(1);
            }
            AsCPangya2TitleMain(self)->textureResource = 0;
        }
        if (self) __dt__18NativeBase80201D18Fv(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Poll the multiplayer menu base until its resources are ready.
int CMultiPlayTopMenu_PollLoadComplete(void* self) {
    return CTopMenuBase_PollLoad(&AsCMultiPlayTopMenu(self)->secondaryVtable); // original +0x18
}

// Update the multiplayer layout.
void CMultiPlayTopMenu_UpdateLayout(void* self) {
    Layout_Update(AsCMultiPlayTopMenu(self)->layout);
}

// Initialize the multiplayer module angle constant to one full turn.
void MultiplayerTopMenu_InitFullTurnConstant(void) {
    s_multiplayerTopMenuTwoPiStorage = kFloat_6_2831855;
}

// Adjust the secondary menu interface pointer back to its owning object before selection.
int CMultiPlayTopMenu_HandleSelectionThunk24(void* self) {
    return CMultiPlayTopMenu_HandleSelection((char*)self - 0x18);
}

// Adjust the secondary menu interface pointer back to its owner before destruction.
void* CMultiPlayTopMenu_DtorThunk24(void* self, int n) {
    return CMultiPlayTopMenu_dtor((char*)self - 0x18, n);
}

// Update the submenu layout.
void CTopSubMenu_UpdateLayout(void* self) {
    Layout_Update(AsCTopSubMenu(self)->layout);
}

// Adjust the secondary submenu interface pointer back to its owner before selection.
int CTopSubMenu_HandleSelectionThunk24(void* self) {
    return CTopSubMenu_HandleSelection((char*)self - 0x18);
}

// Adjust the secondary submenu interface pointer back to its owner before destruction.
void* CTopSubMenu_DtorThunk24(void* self, int n) {
    return CTopSubMenu_dtor((char*)self - 0x18, n);
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624DC", _ctors_u802BC3F8_0, MultiplayerTopMenu_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804624DC", _ctors_u802BC3F8_1, TopSubMenu_InitFullTurnConstant);
#pragma pop
