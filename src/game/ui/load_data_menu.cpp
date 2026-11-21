#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Save/load menu: four cached save records, hover/name/play-time details, slot
// activation feedback and modal delete confirmation. This menu reports intent;
// it does not itself delete/load the persistent slot. Caller consumes result
// with selectedSlot. Ctor/dtor and archive ownership are in darts_dialog_and_load_data_menu.cpp.
// Edit index: slot/name/outfit schema -> LoadMenuRecord below; play-time units ->
// ShowSlotPlayTime (30 saved ticks/second, independent of menu update cadence);
// input/result transitions -> ProcessInput/ProcessSlotSelection; geometry ->
// ResetSlotLayout; BP names/scripts/quads -> u80338838_data.cpp and group arrays.
// Feedback waits 3 ProcessInput calls that pass fade/modal gates; fades use the
// FadeState API's frame count (15). Changing presentation FPS needs both reviewed.
extern "C" {
extern unsigned char s_loadMenuCaptionGroupNames[]; // Load-menu caption group names.
}
// Translation unit 0x80338838-0x80339F44 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadDataMenu.h"
#include "classes/CPangyaCommonDialogFields.h"

#define AsCLoadDataMenu(p) ((CLoadDataMenu*)(p))

enum DeleteConfirmationResult { kDeleteConfirmed = 1, kDeleteCancelled = 2 };
enum LoadMenuResult { kLoadNoResult = 0, kLoadCancelled = 1, kLoadDeleteConfirmed = 2, kLoadSlotChosen = 3 };
enum LoadMenuLayout {
    kSaveSlotCount = 4, kHoveredSlotBase = 4, kActivatedSlotBase = 8,
    kLastActivatedSlot = 11, kSlotFeedbackFrames = 3, kNoSlotFeedback = -1,
    kSlotDisabledState = 3, kSlotPressedState = 2, kSlotInteractionFlag = 5,
    kLoadMenuScriptIndex = 24, kMenuFadeFrames = 15,
    kActionBack = 6, kActionDeleteSlot = 0x26, kSlotNameGroup = 0x3FD,
    kPlayHoursText = 0x22, kPlayMinutesText = 0x21, kPlaySecondsText = 0x20,
    kSaveTicksPerSecond = 30, kSaveTicksPerMinute = 1800, kSaveTicksPerHour = 108000
};
// Cached native save payload (same 0x828-byte record as SavePlayerRecord.h).
// Eight character equipment records carry availability in byte bit6. Other bits
// and equipment bytes are copied intact and are not interpreted by this menu.
struct LoadMenuCharacterEquipment { unsigned char flags; char equipment[0x33]; };
struct LoadMenuRecord {
    unsigned short name[16];
    int playTicks;
    char otherSaveData[0x2CC - 0x24];
    LoadMenuCharacterEquipment characters[8];
    char remainingData[0x828 - 0x46C];
};
struct LoadMenuCachedRecordsView { char menuPrefix[0x38]; LoadMenuRecord records[4]; };
typedef char LoadMenuRecord_size[(sizeof(LoadMenuRecord) == 0x828) ? 1 : -1];
enum LoadMenuRecordLayout {
    kSaveRecordSize = sizeof(LoadMenuRecord),
    kCachedRecordsOffset = (int)&((LoadMenuCachedRecordsView*)0)->records,
    kCachedPlayTicksOffset = kCachedRecordsOffset + (int)&((LoadMenuRecord*)0)->playTicks,
    kOutfitEntrySize = sizeof(LoadMenuCharacterEquipment),
    kCachedOutfitFlagsOffset = kCachedRecordsOffset + (int)&((LoadMenuRecord*)0)->characters
};
// Cache address map: record(index) = self + kCachedRecordsOffset + index*0x828.
// The shared header's legacy +0x38 member spelling is an alias of records[0].name[0];
// its replacement schema is this four-record view, not an occupancy array.
// A first UTF-16 code unit of zero means empty; name buffer must be terminated
// within its 16 code units before Text_SetContent. playTicks is record+0x20.
// Character flags: self+0x304+slot*0x828+character*0x34 = record+0x2CC.
// Layout IDs: slot i=0..3, hovered i+4, activated hit rectangle i+8. Copy the
// currently visible slot rectangle to i+8 when switching normal/hover feedback.
// The final nine entries are BP group-name pointers; the intervening pool stays opaque.
struct LoadMenuGroupTable {
    void* slotGroups[12];
    void* characterGroups[8];
    void* selectionGroups[4];
    char interveningData[0xB4];
    void* titleGroupNames[9];
};
typedef char LoadMenuGroupTable_titleOffset[((int)&((LoadMenuGroupTable*)0)->titleGroupNames == 0x114) ? 1 : -1];
struct SaveLoadMenuStrings {
    char layout[13], bpFile[13], titleCaptionBp[21], archive[14];
    char numberAtlas[16], menuBgm[9], saveLoadScript[10], titleScript[6], padding[6];
};
typedef char SaveLoadMenuStrings_size[(sizeof(SaveLoadMenuStrings) == 108) ? 1 : -1];


extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Arm cleanup for the temporary vector of activated layout IDs.
    explicit ActivatedUnwindCleanup(volatile void* vector)
        : words((volatile unsigned int*)vector), active(true) {}
    // Disarm unwind cleanup before the explicit normal-path free.
    void release() { active = false; }
    // Free activated-ID storage when an operation exits without disarming cleanup.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};


extern float kFloat_6_2831855;
extern float s_loadDataMenuRuntimeTau;
extern float kLoadDataMenuOpacity;
extern signed char s_saveDataInitialized;
extern unsigned char s_saveData[];
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern void* g_soundManager;
extern unsigned char s_loadDataMenuPlayTimeTextEntries[];
extern unsigned char s_bpCtrlHandleDtorRegistration_DartsDialog[];
extern signed char s_fadeOverlayInitialized;
extern unsigned char s_bpCtrl[];
extern unsigned char s_fadeOverlay[];
extern unsigned char s_bpCtrlDtorRecord_SaveLoad[];
extern unsigned char s_fadeOverlayDtorRegistration_80338838[];
extern unsigned char s_loadDataMenuBackgroundQuadTemplate[];
extern unsigned char s_loadDataMenuOverlayQuadTemplate[];
extern unsigned char s_loadDataMenuDeleteConfirmationText[];
extern unsigned char s_gameInputManager[];
extern unsigned char s_inputManagerDtorRecord_LoadDataMenu[];
extern signed char s_inputManagerInitialized;
extern float kLoadDataMenuDialogUpdateStep;
int CPackTextureResource_IsLoadFinished(void*);
void __ct__7CBpCtrlFv(void*);
int CBpCtrl_PollAllEntriesLoaded(void*);
int Layout_PollLoadComplete(void*);
void SaveData_Ctor(void*);
void* SaveData_IsSlotUsed(void*, int);
void* SaveData_GetSlotRecord(void*, int);
void CFontNumDraw_RegisterEntries(void*, void*, int, void*, void*);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void SoundManager_RequestBgmChange(void*, void*, int, int);
void CBpCtrlHandle_Dtor(void*, int);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_StartFadeIn(void*, int);
int FadeState_GetState(void*);
void Layout_Update(void*);
int Layout_HitTestPlayer(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor();
int Input_IsActionPressed(void*, int, int);
int Layout_HitTestElementForSelectedPlayer(void*, int);
void* __nw__FUl(unsigned long);
void __ct__19CPangyaCommonDialogFPvi(void*, int, int);
void CPangyaCommonDialog_SetText(void*, void*);
void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeOut(void*, int);
int Layout_ReadElementRect(void*, int, int*);
void Layout_WriteElementRect(void*, int, int*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void* UIGroup_GetOrCreateById(void*, void*);
void UIGroup_SetEnabled(void*, int);
void CMsgDraw_ResetTextState(void*);
void NumericTextTable_ClearEntryText(void*, int);
int Layout_CollectActivated(void*, void*, void*);
int CLoadDataMenu_InitBpGroups(void*);
int CLoadDataMenu_UpdateSlotDetails(void*, int);
int CLoadDataMenu_ClearSlotDetails(void*);
int CLoadDataMenu_ProcessSlotSelection(void*);
int CLoadDataMenu_ResetSlotLayout(void*);
void appFreeUni__FPv(void*);
extern void* s_loadDataMenuCharacterGroups[];
extern void* s_loadDataMenuSlotSelectionGroups[];
extern LoadMenuGroupTable s_loadDataMenuBpGroupTable;
extern SaveLoadMenuStrings saveLoadMenuStrings;
extern void* loadDataMenuGroupIds;
void* BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void* UITextTable_SetInteger(void*, int, int);
void CBpCtrl_dtor();
void* IdMap_Lookup(void*, void*);
void UIGroup_GetPosition(void*, void*);
void Text_SetPosition(void*, void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void Text_SetContent(void*, void*);
void* BpSprite_GetTextureDescription(void*);
void* GX_GetPrimitiveCount(int, int, float*);
void Engine_SetFlexibleVertexFormat(int);
void DrawPrimitiveUP(int, void*, void*, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void UITextTable_Draw(void*);
void Text_DrawGlyphs(void*);

class X5 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual int v18();
};
// Wait for the archive, BP entries, layout, and background texture to become ready.
int CLoadDataMenu_PollLoadComplete(void* self) {
    if (AsCLoadDataMenu(self)->packTextureResource != 0 &&
        CPackTextureResource_IsLoadFinished(AsCLoadDataMenu(self)->packTextureResource) == 0) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) == 0) return 0;
    if (Layout_PollLoadComplete(AsCLoadDataMenu(self)->layout) == 0) return 0;
    void* obj = AsCLoadDataMenu(self)->bpTexture;
    int c = ((X5*)obj)->v18();
    return c != 0;
}

// Lazily initialize the shared BP script handle and its global destructor registration.
static inline void* GetBpCtrlHandle() {
    void* p = Sing_805DC5F8();
    if (p == 0) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_DartsDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = (void*)s_bpCtrlHandle;
    }
    return Sing_805DC5F8();
}

#pragma use_lmw_stmw on
// Cache occupied save slots, initialize menu groups, select its script and BGM, and start fading.
void CLoadDataMenu_EnterMenu(void* self) {
    // Phase 1: release the completed pack loader, then copy occupied save records
    // into the inline cache. Empty slots retain their already initialized cache.
    struct Deletable {
        virtual ~Deletable() DECOMP_LOCAL_VIRTUAL;
    };
    Deletable* p = (*(Deletable**)&AsCLoadDataMenu(self)->packTextureResource);
    if (p) {
        delete p;
        AsCLoadDataMenu(self)->packTextureResource = 0;
    }

    struct SaveSlot {
        int header[151];
        int info[28];
        unsigned short text[208];
        int values[16];
        unsigned char bytesA[24];
        unsigned char bytesB[6];
        unsigned char bytesC[11];
        int moreValuesA[2];
        int moreValuesB[2];
        int lastValue;
        unsigned short tail[414];
    };
    int i = 0;
    char* base = (char*)self;
    do {
        if (Sing_805DC5E0() == 0) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            Sing_805DC5E0() = s_saveData;
        }
        if (SaveData_IsSlotUsed(Sing_805DC5E0(), i) != 0) {
            if (Sing_805DC5E0() == 0) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                Sing_805DC5E0() = s_saveData;
            }
            SaveSlot* src = (SaveSlot*)SaveData_GetSlotRecord(Sing_805DC5E0(), i);
            SaveSlot* dst = (SaveSlot*)(base + kCachedRecordsOffset);
            *dst = *src;
        }
        ++i;
        base += kSaveRecordSize;
    } while (i < 4);

    // Phase 2: initialize BP groups/hit rectangles and register play-time numbers.
    CLoadDataMenu_InitBpGroups(self);
    CLoadDataMenu_ResetSlotLayout(self);

    // CFontNumDraw_RegisterEntries call
    {
        extern SaveLoadMenuStrings saveLoadMenuStrings;
        void* r3 = AsCLoadDataMenu(self)->textTable;
        void* r6 = AsCLoadDataMenu(self)->bpObject;
        void* r7 = (void*)(saveLoadMenuStrings.numberAtlas);
        void* r4 = (void*)s_loadDataMenuPlayTimeTextEntries;
        int r5 = 3;
        CFontNumDraw_RegisterEntries(r3, r4, r5, r6, r7);
    }

    // Phase 3: save the previous shared script index, switch to the load-menu
    // script/BGM, and start the global fade. The shared handle remains borrowed.
    GetBpCtrlHandle();
    void* q = Sing_805DC5F8();
    bool missing = q == 0;
    AsCLoadDataMenu(self)->savedScriptIndex = ((int*)q)[1];
    if (missing) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_DartsDialog);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = (void*)s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kLoadMenuScriptIndex);

    SoundManager_RequestBgmChange(g_soundManager, (void*)(saveLoadMenuStrings.menuBgm), 0, 1);

    // Lazily initialize the shared fade overlay.
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_80338838);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeOut(Sing_805DC608(), kMenuFadeFrames);
}

struct DeleteConfirmationDialogObject {
    virtual ~DeleteConfirmationDialogObject();
};

struct DeleteConfirmationDialogUpdate {
    virtual void v0();
    virtual void v1();
    virtual int Update(float);
};

struct DeleteConfirmationDialogStorage {
    // Construct the owned common dialog with its type code and fade-frame count.
    DeleteConfirmationDialogStorage(int a, int b) { __ct__19CPangyaCommonDialogFPvi(this, a, b); }
    char data[0x44];
};

#pragma use_lmw_stmw reset
// Transition map (all states keep BP/layout/text resources owned by the menu):
// fade!=0 -> wait, no dialog/feedback tick; finished -> report 1;
// dialog!=null -> modal Update only, even with selectionDelayFrames pending;
//   not ready/unknown result -> keep modal; yes(1) -> result2, finished;
//   cancel(2) -> retain result0, release modal. Both return0 this call.
// delay!=-1 -> decrement once; still>0 -> wait; reaches0 -> restore slot geometry,
//   set delay=-1, latch finished if result!=0, otherwise re-enable interaction;
//   expiry also consumes this call. The deleted modal's delay resumes next call.
// idle -> hover details, then back(result1/finished), activation(result3/delay3),
//   or delete(occupied slot only: delay3 + owned modal), in that priority order.
// No persistent save writes occur. finished is observed only after fades end;
// UpdateSlotDetails on an empty slot clears details but retains its slot marker.
// vfunc_6(enabled) requests fade-in(0); disabling requests the pending transition
// only once finished. Draw always layers the modal last, independently of gates.
// Update hover/pressed feedback, slot selection, cancellation, and the modal delete confirmation.
// Return 1 only when finished was already latched at entry (after fade gating).
// Newly completed operations return 0 this frame; the caller observes 1 next time.
// deleteConfirmationDialog is an owned CPangyaCommonDialog, distinct from the
// borrowed global FadeState. It is updated modally and deleted on yes or cancel.
int CLoadDataMenu_ProcessInput(void* self) {
    int slot28[4];
    int slot18[4];
    int slot8[4];
    int i;

    // Phase 1: global fade blocks all menu/dialog input; finished is checked next.
    if (Sing_805DC608() == 0) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_80338838);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = (void*)s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()) != 0) {
        return 0;
    }
    if (AsCLoadDataMenu(self)->finished != 0) {
        return 1;
    }

    // Phase 2: an active modal consumes the entire update. Confirm latches result
    // 2/finished; cancel merely releases it. Both use the same destruction path.
    if (AsCLoadDataMenu(self)->deleteConfirmationDialog != 0) {
        // 0x80338E00: dialog readiness and result share the return below.
        do {
            if (AsCLoadDataMenu(self)->deleteConfirmationDialog == 0) break;
            if (((DeleteConfirmationDialogUpdate*)AsCLoadDataMenu(self)->deleteConfirmationDialog)->Update(kLoadDataMenuDialogUpdateStep) == 0) break;
            switch (AsCPangyaCommonDialogFields(AsCLoadDataMenu(self)->deleteConfirmationDialog)->result) { // 0x80338E00: dialog +0x8
            case kDeleteConfirmed:
                AsCLoadDataMenu(self)->result = kLoadDeleteConfirmed;
                AsCLoadDataMenu(self)->finished = 1;
                // Fall through to release the completed dialog.
            case kDeleteCancelled:
                if (AsCLoadDataMenu(self)->deleteConfirmationDialog != 0) {
                    delete (DeleteConfirmationDialogObject*)AsCLoadDataMenu(self)->deleteConfirmationDialog;
                    AsCLoadDataMenu(self)->deleteConfirmationDialog = 0;
                }
                break;
            default:
                break;
            }
        } while (0);
        return 0;
    }

    // Phase 3: synchronize normal/hover geometry, then drain pressed feedback.
    // Until the three-frame delay ends, selection and delete input remain gated.
    Layout_Update(AsCLoadDataMenu(self)->layout);

    if (AsCLoadDataMenu(self)->selectionDelayFrames == kNoSlotFeedback) {
        for (i = 0; i < kSaveSlotCount; ++i) {
            if (Layout_HitTestPlayer(AsCLoadDataMenu(self)->layout, i) != 0) {
                Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, i, 0);
                Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase, 1);
                Layout_ReadElementRect(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase, slot18);
                Layout_WriteElementRect(AsCLoadDataMenu(self)->layout, i + kActivatedSlotBase, slot18);
            } else if (Layout_HitTestPlayer(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase) == 0) {
                Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, i, 1);
                Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase, 0);
                Layout_ReadElementRect(AsCLoadDataMenu(self)->layout, i, slot28);
                Layout_WriteElementRect(AsCLoadDataMenu(self)->layout, i + kActivatedSlotBase, slot28);
            }
        }
    }

    int flag;
    if (AsCLoadDataMenu(self)->selectionDelayFrames == kNoSlotFeedback) {
        flag = 0;
    } else {
        int n = AsCLoadDataMenu(self)->selectionDelayFrames - 1;
        AsCLoadDataMenu(self)->selectionDelayFrames = n;
        if (n > 0) {
            flag = 1;
        } else {
            int idx = AsCLoadDataMenu(self)->selectedSlot;
            AsCLoadDataMenu(self)->selectionDelayFrames = kNoSlotFeedback;
            Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, idx, 1);
            Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, idx + kHoveredSlotBase, 0);
            Layout_ReadElementRect(AsCLoadDataMenu(self)->layout, idx, slot8);
            Layout_WriteElementRect(AsCLoadDataMenu(self)->layout, idx + kActivatedSlotBase, slot8);
            Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, AsCLoadDataMenu(self)->selectedSlot + 8, 0);
            if (AsCLoadDataMenu(self)->result != kLoadNoResult) {
                AsCLoadDataMenu(self)->finished = 1;
            } else {
                Layout_SetElementPlayerFlag(AsCLoadDataMenu(self)->layout,
                AsCLoadDataMenu(self)->selectedSlot + 8, 1, 5);
            }
            flag = 1;
        }
    }
    if (flag != 0) return 0;

    // Phase 4: refresh hovered details, then handle back before slot activation
    // and delete. A successful earlier action prevents later actions this frame.
    int j = 0;
    while (1) {
        if (Layout_HitTestElementForSelectedPlayer(AsCLoadDataMenu(self)->layout, j + kActivatedSlotBase) != 0) {
            CLoadDataMenu_UpdateSlotDetails(self, j);
            break;
        }
        if (++j >= 4) {
            CLoadDataMenu_ClearSlotDetails(self);
            break;
        }
    }

    // lazy init input manager
    if (Sing_805DC5C8() == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRecord_LoadDataMenu);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = (void*)s_gameInputManager;
    }
    int pressed = Input_IsActionPressed(Sing_805DC5C8(), kActionBack, -1);
    int ret;
    if (pressed == 0) {
        ret = 0;
    } else {
        AsCLoadDataMenu(self)->finished = 1;
        AsCLoadDataMenu(self)->result = kLoadCancelled;
        ret = 1;
    }
    do {
        if (ret != 0) break;

        if (CLoadDataMenu_ProcessSlotSelection(self) != 0) break;

        if (Sing_805DC5C8() == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRecord_LoadDataMenu);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = (void*)s_gameInputManager;
        }
        if (Input_IsActionPressed(Sing_805DC5C8(), kActionDeleteSlot, -1) == 0) break;

        char* row = (char*)self;
        for (i = 0; i < kSaveSlotCount; ++i) {
            int cond;
            if (i < 0 || i >= kSaveSlotCount) {
                cond = 0;
            } else {
                unsigned short h = ((LoadMenuCachedRecordsView*)row)->records[0].name[0];
                int neg = -h;
                int o = neg | h;
                cond = (unsigned int)o >> 31;
            }
            if (cond != 0) {
                if (Layout_HitTestElementForSelectedPlayer(AsCLoadDataMenu(self)->layout, i) != 0) {
                    Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase, 2);
                    Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, i + kActivatedSlotBase, 2);
                    Layout_SetElementPlayerFlag(AsCLoadDataMenu(self)->layout, i + kHoveredSlotBase, 0, 5);
                    Layout_SetElementPlayerFlag(AsCLoadDataMenu(self)->layout, i + kActivatedSlotBase, 0, 5);
                    AsCLoadDataMenu(self)->selectedSlot = i;
                    AsCLoadDataMenu(self)->selectionDelayFrames = kSlotFeedbackFrames;
                    DeleteConfirmationDialogStorage* obj = new DeleteConfirmationDialogStorage(2, kMenuFadeFrames);
                    AsCLoadDataMenu(self)->deleteConfirmationDialog = obj;
                    CPangyaCommonDialog_SetText(obj, s_loadDataMenuDeleteConfirmationText);
                    break;
                }
            }
            row += kSaveRecordSize;
        }

    } while (0);
    return 0;
}

// Request immediate fade-in when enabled, or the pending transition when finished.
// This hook does not restore interaction flags; ProcessInput's delay expiry does.
void CLoadDataMenu_StartFadeOrRequestTransition(void* self, int enabled) {
    if (enabled) {
        if (Sing_805DC608() == 0) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_80338838);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_StartFadeIn(Sing_805DC608(), 0);
    } else if (AsCLoadDataMenu(self)->finished) {
        if (Sing_805DC608() == 0) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_80338838);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        FadeState_RequestFramebufferCapture(Sing_805DC608());
    }
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
// Draw background quads, menu resources, slot details, and the active confirmation dialog.
void CLoadDataMenu_Draw(void* self) {
    // Phase 1: bind the background texture and rebuild two 4-vertex quads from
    // templates. Each vertex is 24 bytes; texture dimensions size their XY corners.
    unsigned int* save;
    save = (unsigned int*)BpSprite_GetTextureDescription(AsCLoadDataMenu(self)->bpTexture);
    ((void (*)(void*))(*(void***)AsCLoadDataMenu(self)->bpTexture)[5])(AsCLoadDataMenu(self)->bpTexture);

    unsigned char verts2[0x60];
    unsigned char verts[0x60];
    int n = 12;
    int* src = (int*)s_loadDataMenuBackgroundQuadTemplate - 1;
    int* dst = (int*)verts - 1;
    do {
        int a = src[1];
        src += 2;
        int b = src[0];
        dst[1] = a;
        dst += 2;
        dst[0] = b;
    } while (--n);
    ((float*)verts)[6] = (float)save[0];
    ((float*)verts)[13] = (float)save[1];
    ((float*)verts)[18] = (float)save[0];
    ((float*)verts)[19] = (float)save[1];
    void* buffer = GX_GetPrimitiveCount(4, 0x98, (float*)dst);
    Engine_SetFlexibleVertexFormat(0x11F005);
    DrawPrimitiveUP(0x98, buffer, verts, 0x18);
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);

    n = 12;
    src = (int*)s_loadDataMenuOverlayQuadTemplate - 1;
    dst = (int*)verts2 - 1;
    do {
        int a = src[1];
        src += 2;
        int b = src[0];
        dst[1] = a;
        dst += 2;
        dst[0] = b;
    } while (--n);
    ((float*)verts2)[6] = (float)save[0];
    ((float*)verts2)[13] = (float)save[1];
    ((float*)verts2)[18] = (float)save[0];
    ((float*)verts2)[19] = (float)save[1];
    buffer = GX_GetPrimitiveCount(4, 0x98, (float*)dst);
    Engine_SetFlexibleVertexFormat(0x11F005);
    DrawPrimitiveUP(0x98, buffer, verts2, 0x18);

    // Phase 2: draw the main and title BP objects, then layout and detail text.
    BpObject_TryInvokeCurrentCallback(AsCLoadDataMenu(self)->bpObject, 0);
    BpObject_Draw(AsCLoadDataMenu(self)->bpObject);
    Layout_DrawWithOpacity(AsCLoadDataMenu(self)->layout, kLoadDataMenuOpacity);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bp = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsCLoadDataMenu(self)->secondaryBpEntryId));
    BpObject_TryInvokeCurrentCallback(bp, 0);
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    bp = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&AsCLoadDataMenu(self)->secondaryBpEntryId));
    BpObject_Draw(bp);
    UITextTable_Draw(AsCLoadDataMenu(self)->textTable);
    Text_DrawGlyphs(AsCLoadDataMenu(self)->messageDraw);
    // Phase 3: the owned modal dialog draws above the completed menu layers.
    void* object = AsCLoadDataMenu(self)->deleteConfirmationDialog;
    if (object) (*(void (**)(void*))(*(char**)object + 0x14))(object);
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

// Align slot hit rectangles and mark empty cached slots as disabled.
int CLoadDataMenu_ResetSlotLayout(void* self) {
    int values[4];
    char* p = (char*)self;
    for (int i = 0; i < kSaveSlotCount; ++i) {
        Layout_ReadElementRect(AsCLoadDataMenu(self)->layout, i, values);
        Layout_WriteElementRect(AsCLoadDataMenu(self)->layout, i + kActivatedSlotBase, values);
    }
    for (int i = 0; i < kSaveSlotCount; ++i) {
        int cond;
        if (i < 0 || i >= kSaveSlotCount) {
            cond = 0;
        } else {
            unsigned short h = *(unsigned short*)(p + kCachedRecordsOffset);
            int neg = -h;
            int o = neg | h;
            cond = (unsigned int)o >> 31;
        }
        if (!cond) {
            Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, i, 3);
            Layout_SetElementFeedbackState(AsCLoadDataMenu(self)->layout, i, 3);
        }
        p += kSaveRecordSize;
    }
    for (p = (char*)4; (int)p <= 7; p = (char*)((int)p + 1)) Layout_SetElementPlayerFlagsAndState(AsCLoadDataMenu(self)->layout, (int)p, 0);
    return 1;
}

// Hide character/time groups and clear the cached slot-name message.
int CLoadDataMenu_ClearSlotDetails(void* self) {
    int i = 0;
    void** p = s_loadDataMenuCharacterGroups;
    while (i < 8) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, *p);
        UIGroup_SetEnabled(a, 0);
        ++i;
        ++p;
    }
    CMsgDraw_ResetTextState(AsCLoadDataMenu(self)->messageDraw);
    NumericTextTable_ClearEntryText(AsCLoadDataMenu(self)->textTable, kPlayHoursText);
    NumericTextTable_ClearEntryText(AsCLoadDataMenu(self)->textTable, kPlayMinutesText);
    NumericTextTable_ClearEntryText(AsCLoadDataMenu(self)->textTable, kPlaySecondsText);
    i = 0;
    void** names = &loadDataMenuGroupIds;
    while ((unsigned int)i < 2) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, *names);
        UIGroup_SetEnabled(a, 0);
        ++i;
        ++names;
    }
    for (int i = 0; i < 4; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, s_loadDataMenuSlotSelectionGroups[i]);
        UIGroup_SetEnabled(a, 0);
    }
    return 1;
}

// Latch an activated save slot and start its pressed-feedback delay.
// Returns whether Layout_CollectActivated reported any activations. It still
// latches kLoadSlotChosen if none of those IDs lies in 8..11; valid layouts are
// expected to report slot activations here. The temporary ID allocation is owned
// locally and freed on both normal exits and unwind.
int CLoadDataMenu_ProcessSlotSelection(void* self) {
    struct List { int* items; int count; int capacity; } list = {0, 0, 0};
    ActivatedUnwindCleanup activatedCleanup(&list);
    int result = Layout_CollectActivated(AsCLoadDataMenu(self)->layout, &list, 0);
    if (!result) {
        if (list.items) {
            list.count = 0;
            activatedCleanup.release(); appFreeUni__FPv(list.items);
        }
        return 0;
    }
    for (int i = 0; (unsigned int)i < (unsigned int)list.count; ++i) {
        int id = list.items[i];
        if (id >= kActivatedSlotBase && id <= kLastActivatedSlot) {
            int index = id - kActivatedSlotBase;
            Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, index + kHoveredSlotBase, 2);
            Layout_SetElementValueState(AsCLoadDataMenu(self)->layout, index + kActivatedSlotBase, 2);
            Layout_SetElementPlayerFlag(AsCLoadDataMenu(self)->layout, index + 4, 0, 5);
            Layout_SetElementPlayerFlag(AsCLoadDataMenu(self)->layout, index + 8, 0, 5);
            AsCLoadDataMenu(self)->selectedSlot = index;
            AsCLoadDataMenu(self)->selectionDelayFrames = kSlotFeedbackFrames;
            break;
        }
    }
    AsCLoadDataMenu(self)->result = kLoadSlotChosen;
    if (list.items) {
        list.count = 0;
        activatedCleanup.release(); appFreeUni__FPv(list.items);
    }
    return 1;
}

// Convert the slot's 30-Hz play ticks to hours, minutes, and seconds for display.
// index is a cache slot (0..3), not a render-player index. Invalid slots return 0
// without changing text; valid ones return 1. Nonpositive components clamp to 0.
// The signed counter is at record(index)+0x20; minutes/seconds are remainders.
int CLoadDataMenu_ShowSlotPlayTime(void* self, int index) {
    if (index < 0 || index >= 4) return 0;
    int playTicks = *(int*)((char*)self + index * kSaveRecordSize + kCachedPlayTicksOffset);
    int hours = playTicks / kSaveTicksPerHour;
    if (hours <= 0) hours = 0;
    int minutes = playTicks / kSaveTicksPerMinute - hours * 60;
    if (minutes <= 0) minutes = 0;
    int seconds = playTicks / kSaveTicksPerSecond - hours * 3600 - minutes * 60;
    if (seconds <= 0) seconds = 0;
    UITextTable_SetInteger(AsCLoadDataMenu(self)->textTable, kPlayHoursText, hours);
    UITextTable_SetInteger(AsCLoadDataMenu(self)->textTable, kPlayMinutesText, minutes);
    UITextTable_SetInteger(AsCLoadDataMenu(self)->textTable, kPlaySecondsText, seconds);
    for (unsigned int i = 0; i < 2; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, ((void**)&loadDataMenuGroupIds)[i]);
        UIGroup_SetEnabled(a, 1);
    }
    return 1;
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Show the hovered slot's available characters, name, elapsed time, and slot marker.
int CLoadDataMenu_UpdateSlotDetails(void* self, int index) {
    // Phase 1: reject out-of-range slots; an empty slot clears text/characters
    // but still enables its selection marker and returns success.
    if (index < 0 || index >= kSaveSlotCount) return 0;
    int nz;
    if (index < 0 || index >= kSaveSlotCount) {
        nz = 0;
    } else {
        unsigned short h = *(unsigned short*)((char*)self + index * kSaveRecordSize + kCachedRecordsOffset);
        int neg = -h;
        int o = neg | h;
        nz = (unsigned int)o >> 31;
    }
    if (nz == 0) {
        CLoadDataMenu_ClearSlotDetails(self);
        if (index >= 0 && index < 4) {
            int i = 0;
            void** p = s_loadDataMenuSlotSelectionGroups;
            do {
                void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, *p);
                int eq = (i == index);
                UIGroup_SetEnabled(a, eq);
                ++i;
                ++p;
            } while (i < kSaveSlotCount);
        }
        return 1;
    }

    // Phase 2: one BP group per character index 0..7, shown iff that character's
    // cached equipment byte bit6 is set. This is the same selectability test as
    // CCharaSelect_UpdateSlotInputAvailability (chara_select_screen_part3.cpp, save+0x2CC).
    // The sign test below extracts bit6; adjacent bit5 does not affect it.
    // It does NOT test the current equipped character at save+0x2C, and does
    // not show one group per outfit. Empty slots take the earlier clear path.
    void* marker;
    void** bb;
    unsigned char* row;
    int mull;
    int i;
    mull = index * kSaveRecordSize;
    bb = s_loadDataMenuCharacterGroups;
    i = 0;
    row = (unsigned char*)self + mull + kCachedOutfitFlagsOffset;
    do {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, *bb);
        int enabled;
        if (index < 0 || index >= kSaveSlotCount) {
            enabled = 0;
        } else if (i < 0 || i >= 8) {
            enabled = 0;
        } else {
            unsigned char b = *row;
            unsigned int t = (unsigned int)b << 25;
            enabled = ((int)(t & 0xC0000000) >> 31) != 0;
        }
        UIGroup_SetEnabled(a, enabled);
        ++i;
        ++bb;
        row += kOutfitEntrySize;
    } while (i < 8);
    // Phase 3: position the cached name at its BP marker, fill play-time digits,
    // and enable exactly the hovered slot's selection group.
    marker = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, (void*)kSlotNameGroup);
    if (marker != 0) {
        CMsgDraw_ResetTextState(AsCLoadDataMenu(self)->messageDraw);
        if (marker != 0) {
            char rect[16];
            UIGroup_GetPosition(marker, rect);
            Text_SetPosition(AsCLoadDataMenu(self)->messageDraw, rect);
        }
        CMsgDraw_ClearFormattingAndRevealAll(AsCLoadDataMenu(self)->messageDraw);
        Text_SetContent(AsCLoadDataMenu(self)->messageDraw, (char*)self + mull + kCachedRecordsOffset);
    }
    CLoadDataMenu_ShowSlotPlayTime(self, index);
    if (index >= 0 && index < 4) {
        int i2 = 0;
        void** p = s_loadDataMenuSlotSelectionGroups;
        do {
            void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, *p);
            int eq = (i2 == index);
            UIGroup_SetEnabled(a, eq);
            ++i2;
            ++p;
        } while (i2 < 4);
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Start the save/load and title scripts with all detail groups initially hidden.
int CLoadDataMenu_InitBpGroups(void* self) {
    void** base;
    unsigned int i;
    void** names;
    struct Nine { int v[9]; };
    Nine local;
    void* obj;
    base = (void**)&s_loadDataMenuBpGroupTable;
    BpObject_StartScriptByName(AsCLoadDataMenu(self)->bpObject, saveLoadMenuStrings.saveLoadScript, 1);
    for (i = 0; i < 12; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, base[i]);
        UIGroup_SetEnabled(a, 0);
    }
    for (i = 0; i < 8; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, base[12 + i]);
        UIGroup_SetEnabled(a, 0);
    }
    for (i = 0; i < 4; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, base[20 + i]);
        UIGroup_SetEnabled(a, 0);
    }
    names = (void**)&loadDataMenuGroupIds;
    for (i = 0; i < 2; ++i) {
        void* a = UIGroup_GetOrCreateById(AsCLoadDataMenu(self)->bpObject, names[i]);
        UIGroup_SetEnabled(a, 0);
    }
    local = *(Nine*)&((LoadMenuGroupTable*)base)->titleGroupNames;
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_SaveLoad);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    obj = IdMap_Lookup(Sing_805DC5F0(), (void*)AsCLoadDataMenu(self)->secondaryBpEntryId);
    BpObject_StartScriptByName(obj, saveLoadMenuStrings.titleScript, 1);
    for (i = 0; i < 9; ++i) {
        void* a = BpObject_GetOrCreateNamedGroup(obj, (void*)local.v[i]);
        UIGroup_SetEnabled(a, 0);
    }
    return 1;
}

// Initialize the load-menu unit's runtime full-circle constant.
void CLoadDataMenu_InitRuntimeTau() { s_loadDataMenuRuntimeTau = kFloat_6_2831855; }

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462658", _ctors_u80338838_0, CLoadDataMenu_InitRuntimeTau);
#pragma pop
