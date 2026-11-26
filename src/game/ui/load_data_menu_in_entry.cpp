// Load-data menu used on the entry screen: chooses one of four save slots, or the fifth slot to
// load from memory. Caches the slot records and shows the details of the hovered slot.

extern "C" {
extern unsigned char kSaveSlotDetailGroupNames[];
extern unsigned char s_saveLoadEntryCaptionGroupNames[]; // Save/load entry caption group names.
extern unsigned char loadDataSlotGroupIds[];
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CLoadDataMenuInEntry.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CPadMemoryMenuLoad.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrlHandle.h"

#define AsCLoadDataMenuInEntry(p) ((CLoadDataMenuInEntry*)(p))

enum LoadEntryConstants {
    kNandSlotCount = 4,
    kVisibleSlotCount = 5,
    kMemorySlot = 4,
    kActiveSlotElementBase = 5,
    kSlotDetailElementBase = 10,
    kFirstActiveSlotElement = 5,
    kLastActiveSlotElement = 9,
    kSelectionFeedbackFrames = 3,
    kNoSelection = -1,
    kCancelAction = 6,
    kAllInputPlayers = -1,
    kLoadEntryScriptIndex = 13,
    kSlotDetailsGroupId = 0x3FD,
    kLoadEntryVertexFormat = 0x11F005,
    kTriangleStrip = 0x98,
    kLoadEntryVertexBytes = 0x18
};
// Layout element map for slot i=0..4: i = idle target, i+5 = active/highlight
// target, i+10 = detail/feedback overlay. Slots 0..3 are NAND; slot 4 opens
// memory loading until memoryRecordLoaded. Value states 0/1/2 = idle/hover/pressed.
// selectionDelayFrames=-1 allows hover; selectionComplete is the accepted result.
// Global services repeated below are borrowed, lazily initialized and registered
// for process shutdown; their setup blocks are not per-entry ownership transfers.

// All offsets address entries within the original, externally owned string pool.
struct SaveLoadEntryStringPool {
    char layoutFile[19];
    char menuBpFile[19];
    char captionBpFile[21];
    char textureArchive[14];
    char numberFont[16];
    char menuScript[10];
    char captionScript[6];
};

// Register a typed destructor through the runtime's untyped registration ABI.
inline void __register_global_object(void* object, void (*destructor)(void*, int), void* regmem) {
    __register_global_object(object, (void*)destructor, regmem);
}

struct LoadEntryMenuView;
struct EntryState {
    int entryId, scriptIndex;
    // Read the shared script selection that this menu saves and later restores.
    int value() {
        return scriptIndex;
    }
};
extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Arm array cleanup for an activated-element vector on exceptional exits.
    explicit ActivatedUnwindCleanup(volatile void* vector) : words((volatile unsigned int*)vector), active(true) {}
    // Disarm before explicit normal cleanup frees the same array.
    void release() {
        active = false;
    }
    // Empty/free only the array; its integer element IDs own no objects.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};

void Layout_Update(void*);
int Layout_PollLoadComplete(void*);
void CBpCtrl_dtor(void*, int);
void* GX_GetPrimitiveCount(int, int);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
extern "C" void DrawPrimitiveUP(int, void*, void*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void* IdMap_Lookup(void*, int);
void UITextTable_Draw(void*);
void MPBase1_DrawIfInitialized(void*);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerArray_DeleteValue(void*, void*);
void PairVector_InitReserved(void*);
void PointerPairList_RemoveByKey(void*, void*);
void Layout_Destroy(void*, int);
void PointerVector_DestroyStorage(void*, int);
void MessageDrawPairList_Dtor(void*, int);
void __dl__FPv(void*);
void CBpCtrlHandle_Dtor(void*, int);
void* memcpy(void*, const void*, unsigned int);
void MPBase1_Ctor(void*);
void* __nw__FUl(unsigned int);
void Layout_Init(void*, int);
void Allocation_Dtor(void*, int);
void SaveData_InitPlayerRecord(void*);
void* InputManager_GetSelectedPlayer(void*);
void Layout_LoadExtended(void*, void*, const char*);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void* ResourceTable_FindEntryByName(void*, const char*, int*);
void* SaveData_IsSlotUsed(void*, int);
void* SaveData_GetSlotRecord(void*, int);
void SaveData_Ctor(const char*);
void UIGroup_GetPosition(void*, void*);
void Text_SetPosition(void*, void*);
void* UIGroup_GetOrCreateById(void*, int);
void IFadeMenuBase_StartMenuFadeOut(void*);
int Layout_CollectActivated(void*, int**, int*);
void appFreeUni__FPv(void*);
void Layout_SetElementValueState(void*, int, int);
int Layout_HitTestPlayer(void*, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
int FadeState_GetState(void*);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*, int);
int InputManager_Init(void*);
void InputManager_Dtor(void*, int);
int Input_IsActionPressed(void*, int, int);
int Layout_HitTestElementForSelectedPlayer(void*, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void BpObject_StartScriptByName(void*, const char*, int);
void UIGroup_SetEnabled(void*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];
extern signed char s_bpCtrlHandleInitialized;
extern EntryState s_bpCtrlHandle;
extern char __vt__20CLoadDataMenuInEntry[];
extern char s_bpCtrlDtorRegistration_804E9130[];
static char s_bpCtrlDtorRegistration_LoadDataEntry[12];
static char s_bpCtrlHandleDtorRegistration_80339F44[12];
#pragma force_active on
static char reservedRecords[24];
#pragma force_active off
static char record30[12];
static char record3C[12];
#pragma force_active on
static char record48[12];
#pragma force_active off
extern char saveLoadEntryStrings[];
extern char s_saveLoadEntryGroupTables[];
extern int saveLoadDetailGroupIds;
extern int s_saveLoadEntryBackgroundQuadTemplate[];
extern float kFloat_1_LoadEntryOpacity;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_LoadEntry[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_LoadEntry[];
extern char s_bpCtrlDtorRegistration_LoadDataEntry[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
struct ChatMessageFactoryView {
    char managerStorage[0x10];
    void* textFactory;
};
extern ChatMessageFactoryView* g_chatMsg;
extern signed char s_saveDataInitialized;
extern EntryState s_bpCtrlHandle;
extern char s_saveData[];
extern char s_saveLoadPlayTimeNumberDescriptors[];
extern char s_bpCtrlHandleDtorRegistration_80339F44[];
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[];
extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern signed char s_bpCtrlInitialized;
extern char saveLoadEntryStrings[];
extern signed char s_inputManagerInitialized;
extern ChatMessageFactoryView* g_chatMsg;

struct CachedSaveSlot {
    SavePlayerRecord record;
};
struct LoadEntryMenuView {
    char _0[0x18];
    void* layout;              // 0x18
    void* bpObject;            // 0x1c
    int bpEntryId;             // 0x20
    char _24[0x4];             // 0x24
    CachedSaveSlot entries[5]; // 0x28 .. 0x28F0
    char _28F0[0x8];
    void* textFactory;       // 0x28F8
    void* slotDetailsText;   // 0x28FC
    void* playTimeTextTable; // 0x2900
    void* memoryLoadMenu;    // 0x2904
    void* memoryLoadContext; // 0x2908
    char _290C[0x4];
    void* textureResource;  // 0x2910
    int savedScriptIndex;   // 0x2914
    void* captionBpEntryId; // 0x2918
};
// Ownership: layout, BP entry handles, detail draw, play-time draw, memory menu
// and texture archive are acquired for this entry. textFactory and
// memoryLoadContext are borrowed. Draws must be unregistered before deletion.

struct MenuOwnedObjectCalls {
    virtual void destroy(int);
};
// Unregister text, destroy owned menus/resources, and restore the saved shared script index.
CLoadDataMenuInEntry::~CLoadDataMenuInEntry() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    *(void**)self = __vt__20CLoadDataMenuInEntry;
    // Phase 1: remove draw registrations before deleting their draw objects.
    UITextTable_ClearEntries(self->playTimeTextTable);
    if (self->playTimeTextTable) {
        if (!Sing_805DC830()) {
            if (!s_pointerVectorInitialized) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, PointerVector_DestroyStorage, record30);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), self->playTimeTextTable);
    }
    if (!Sing_805DC650()) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor, record3C);
            s_pointerPairListInitialized = 1;
        }
        Sing_805DC650() = s_messageDrawPairList;
    }
    PointerPairList_RemoveByKey(Sing_805DC650(), self->slotDetailsText);
    if (self->slotDetailsText) {
        if (self->slotDetailsText) {
            ((MenuOwnedObjectCalls*)self->slotDetailsText)->destroy(1);
        }
        self->slotDetailsText = 0;
    }
    // Phase 2: destroy layout and release both BP script-entry handles.
    if (self->layout) {
        Layout_Destroy(self->layout, 1);
        self->layout = 0;
    }
    // handle 0x20

    if (AsCLoadDataMenuInEntry(self)->bpEntryId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                void* object = s_bpCtrl;
                void (*destructor)(void*, int) = CBpCtrl_dtor;
                void* regmem = s_bpCtrlDtorRegistration_LoadDataEntry;
                __register_global_object(object, destructor, regmem);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(AsCLoadDataMenuInEntry(self)->bpEntryId);
        AsCLoadDataMenuInEntry(self)->bpEntryId = -1;
    }
    if ((int)self->captionBpEntryId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                void* object = s_bpCtrl;
                void (*destructor)(void*, int) = CBpCtrl_dtor;
                void* regmem = s_bpCtrlDtorRegistration_LoadDataEntry;
                __register_global_object(object, destructor, regmem);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById((int)(unsigned int)self->captionBpEntryId);
        self->captionBpEntryId = (void*)-1;
    }
    // Phase 3: delete owned child menu/archive, restore the prior shared script.
    if (self->memoryLoadMenu) {
        if (self->memoryLoadMenu) {
            ((MenuOwnedObjectCalls*)self->memoryLoadMenu)->destroy(1);
        }
        self->memoryLoadMenu = 0;
    }
    if (self->textureResource) {
        if (self->textureResource) {
            ((MenuOwnedObjectCalls*)self->textureResource)->destroy(1);
        }
        self->textureResource = 0;
    }
    if (!((EntryState*&)Sing_805DC5F8())) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle.scriptIndex = -1;
            s_bpCtrlHandle.entryId = -1;
            __register_global_object(&s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_80339F44);
            s_bpCtrlHandleInitialized = 1;
        }
        ((EntryState*&)Sing_805DC5F8()) = &s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)(((EntryState*&)Sing_805DC5F8())))->SetScriptIndex(self->savedScriptIndex);
}

// Poll the texture archive and update the loading layout.
void CLoadDataMenuInEntry::PollTextureAndUpdateLayout() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    if (self->textureResource)
        ((CPackTextureResource*)(self->textureResource))->PollLoad();
    Layout_Update(self->layout);
}

// Original 0x8033AEE0: preserve the integer result of the slot-range check.
// Test the first name code unit through a menu cursor advanced by whole save records.
inline int SlotHasRecord(int i, unsigned char* entry) {
    // entry = menu base + i*sizeof(SavePlayerRecord), a synthetic byte cursor.
    // It must only be used for entries[0]; other menu fields would be misaddressed.
    int inRange = 0;
    if (i >= 0 && i < kVisibleSlotCount)
        inRange = (AsSavePlayerRecord(&((LoadEntryMenuView*)entry)->entries[0])->firstNameCodeUnit) != 0;
    return inRange;
}

// Original 0x8033AEE0: stop at the first active detail slot.
// Display details for the first hovered slot or clear the detail panel.
inline void UpdateActiveSlotDetails(LoadEntryMenuView* self, int& i) {
    for (i = 0; i < kVisibleSlotCount; ++i) {
        if (Layout_HitTestElementForSelectedPlayer(self->layout, i + kSlotDetailElementBase)) {
            ((CLoadDataMenuInEntry*)self)->UpdateSlotDetails(i);
            return;
        }
    }
    ((CLoadDataMenuInEntry*)self)->ClearSlotDetails();
}

// Process memory loading, slot hover/selection feedback, cancellation, and slot details after fading.
int CLoadDataMenuInEntry::Update(float delta) {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    // Return 1 to leave this menu (selection completed OR cancel), 0 to keep it.
    // Phase 1: wait for global fade and optional memory-load child before input.
    if (!((void* volatile&)Sing_805DC608())) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_LoadEntry);
            s_fadeOverlayInitialized = 1;
        }
        ((void* volatile&)Sing_805DC608()) = s_fadeOverlay;
    }
    if (FadeState_GetState(((void* volatile&)Sing_805DC608())))
        return 0;
    if (AsCLoadDataMenuInEntry(self)->selectionComplete)
        return 1;
    if (self->memoryLoadMenu) {
        int memoryLoadInProgress = ((CLoadDataMenuInEntry*)self)->UpdateMemoryLoad(delta);
        if (memoryLoadInProgress)
            return 0;
    }
    Layout_Update(self->layout);
    if (AsCLoadDataMenuInEntry(self)->selectionDelayFrames == -1) {
        // Phase 2: move idle/hover states and keep each detail hit rectangle
        // aligned with whichever target is visible. Cached-record cursor moves
        // by record size while layout IDs move by one.
        int i = 0;
        unsigned char* entry = (unsigned char*)self;
        for (; i < kVisibleSlotCount; ++i, entry += sizeof(SavePlayerRecord)) {
            unsigned char idleRect[16];
            unsigned char activeRect[16];
            if (Layout_HitTestPlayer(self->layout, i)) {
                Layout_SetElementPlayerFlagsAndState(self->layout, i, 0);
                Layout_SetElementPlayerFlagsAndState(self->layout, i + kActiveSlotElementBase, 1);
                Layout_SetElementValueState(self->layout, i + kSlotDetailElementBase, 1);
                Layout_ReadElementRect(self->layout, i + kActiveSlotElementBase, activeRect);
                Layout_WriteElementRect(self->layout, i + kSlotDetailElementBase, activeRect);
            } else if (!Layout_HitTestPlayer(self->layout, i + kActiveSlotElementBase)) {
                if ((AsCLoadDataMenuInEntry(self)->memoryRecordLoaded == 0 && i == kMemorySlot) ||
                    SlotHasRecord(i, entry)) {
                    Layout_SetElementPlayerFlagsAndState(self->layout, i, 1);
                    Layout_SetElementPlayerFlagsAndState(self->layout, i + kActiveSlotElementBase, 0);
                    Layout_SetElementValueState(self->layout, i + kSlotDetailElementBase, 0);
                    Layout_ReadElementRect(self->layout, i, idleRect);
                    Layout_WriteElementRect(self->layout, i + kSlotDetailElementBase, idleRect);
                }
            }
        }
    }
    // Phase 3: allow pressed feedback to finish, then handle cancel/activation
    // and finally update the hovered slot's detail panel.
    if (((CLoadDataMenuInEntry*)self)->UpdateSelectionDelay())
        return 0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_LoadEntry);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    int cancelPressed = (Input_IsActionPressed(GetInputManagerSlot(), kCancelAction, kAllInputPlayers) != 0);
    if (cancelPressed)
        return 1;
    if (((CLoadDataMenuInEntry*)self)->ProcessSlotSelection())
        return 0;
    int detailIndex;
    UpdateActiveSlotDetails(self, detailIndex);
    return 0;
}

struct CopyA {
    int w[151];
};
struct CopyB {
    int w[28];
};
struct CopyC {
    short h[208];
};
struct CopyD {
    short h[414];
};
struct CopyE {
    int w[16];
};
struct CopyF1 {
    unsigned char b[24];
};
struct CopyF2 {
    unsigned char b[8];
};
struct CopyH {
    unsigned char b[3];
};
struct CopyG {
    int w[5];
};

// Preserve the save ABI's copy widths and padding; names below identify copy partitions.
// middleSection, auxiliaryWords, status and trailingSection retain uninterpreted payload sections.
// Both copy paths below cover 0x000..0x4D5 and 0x4D8..0x828. Bytes
// 0x4D5..0x4D7 are alignment padding, deliberately not assigned; never replace
// these partitions with memcpy(sizeof(record)) without rechecking that contract.
struct SelectedSaveRecordView {
    CopyA profilePrefix;  // 0x000..0x25C
    CopyB recordBlock25C; // 0x25C..0x2CC copy partition; saved character outfits start at 0x2CC.
    CopyC middleSection;  // 0x2CC..0x46C
    CopyE auxiliaryWords; // 0x46C..0x4AC
    struct StatusChunks {
        CopyF1 first;
        unsigned char scalars[6];
        CopyF2 second;
        CopyH final;
    } status;              // 0x4AC..0x4D5, followed by three alignment bytes
    CopyG additionalWords; // 0x4D8..0x4EC
    CopyD trailingSection; // 0x4EC..0x828
};
typedef char SelectedSaveRecordSize[(sizeof(SelectedSaveRecordView) == sizeof(SavePlayerRecord)) ? 1 : -1];
typedef char SelectedSaveRecordBlock25COffset[((int)&((SelectedSaveRecordView*)0)->recordBlock25C == 0x25C) ? 1 : -1];
typedef char SelectedSaveTailOffset[((int)&((SelectedSaveRecordView*)0)->trailingSection == 0x4EC) ? 1 : -1];

// Copy the selected completed slot payload, preserving the original word/short/byte copy partitions.
int CLoadDataMenuInEntry::CopySelectedRecord(void* out) {
    // out borrows writable record storage. 1 means copied, 0 means incomplete or
    // selectedSlot>=5. A negative selectedSlot is NOT checked; selection logic
    // must have installed 0..4 before marking completion. out's padding survives.
    unsigned char* src = (unsigned char*)this;
    SelectedSaveRecordView* dst = (SelectedSaveRecordView*)out;
    if (!AsCLoadDataMenuInEntry(src)->selectionComplete)
        return 0;
    int selectedSlot = AsCLoadDataMenuInEntry(src)->selectedItem; // 0x80339F44 unit: selection at +0x28F0
    if (selectedSlot >= kVisibleSlotCount)
        return 0;
    src += selectedSlot * sizeof(SavePlayerRecord);

    dst->profilePrefix = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->profilePrefix;
    dst->recordBlock25C = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->recordBlock25C;
    dst->middleSection = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->middleSection;
    dst->auxiliaryWords = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->auxiliaryWords;
    dst->status.first = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.first;
    dst->status.scalars[0] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[0];
    dst->status.scalars[1] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[1];
    dst->status.scalars[2] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[2];
    dst->status.scalars[3] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[3];
    dst->status.scalars[4] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[4];
    dst->status.scalars[5] = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.scalars[5];
    dst->status.second = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.second;
    dst->status.final = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->status.final;
    dst->additionalWords = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->additionalWords;
    dst->trailingSection = ((SelectedSaveRecordView*)&((LoadEntryMenuView*)src)->entries[0])->trailingSection;
    return 1;
}

// Wait for all BP resources and the entry layout to load.
int CLoadDataMenuInEntry::PollLoadComplete() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadDataEntry);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (!((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded())
        return 0;
    return Layout_PollLoadComplete(self->layout) != 0;
}

// Draw the background, slot layout, caption, play-time numbers, and optional memory-load menu.
void CLoadDataMenuInEntry::Draw() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    int vertices[24];
    int i;
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    int* sourceWord = (int*)s_saveLoadEntryBackgroundQuadTemplate - 1;
    int* destinationWord = vertices - 1;
    int remainingWordPairs = 12;
    do {
        int firstWord = sourceWord[1];
        int secondWord = sourceWord[2];
        sourceWord += 2;
        destinationWord[1] = firstWord;
        destinationWord[2] = secondWord;
        destinationWord += 2;
    } while (--remainingWordPairs);
    void* primitiveCount = GX_GetPrimitiveCount(4, kTriangleStrip);
    Engine_SetFlexibleVertexFormat(kLoadEntryVertexFormat);
    DrawPrimitiveUP(kTriangleStrip, primitiveCount, vertices, kLoadEntryVertexBytes);
    BpObject_TryInvokeCurrentCallback(self->bpObject, 0);
    BpObject_Draw(self->bpObject);
    Layout_DrawWithOpacity(self->layout, kFloat_1_LoadEntryOpacity);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadDataEntry);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bp = IdMap_Lookup(Sing_805DC5F0(), (int)(unsigned int)self->captionBpEntryId);
    BpObject_TryInvokeCurrentCallback(bp, 0);
    UITextTable_Draw(self->playTimeTextTable);
    if (self->memoryLoadMenu)
        MPBase1_DrawIfInitialized(self->memoryLoadMenu);
}
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Accept activated slot elements; open memory loading for the empty fifth slot or record the selected slot.
int CLoadDataMenuInEntry::ProcessSlotSelection() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    // The temporary vector owns only its array of activated integer element IDs;
    // it borrows no save records. Layout_CollectActivated fills data/count/capacity.
    // Return 0 only when collection reports no activation; 1 also covers a list
    // containing no valid slot ID. selectedItem changes only for a valid slot.
    int byteOffset = 0;
    struct List {
        volatile int* data;
        unsigned int count;
        int capacity;
    } list;
    list.data = 0;
    list.count = 0;
    list.capacity = 0;
    ActivatedUnwindCleanup activatedCleanup(&list);
    int found = Layout_CollectActivated(self->layout, (int**)&list, 0);
    if (!found) {
        if (list.data) {
            volatile unsigned int* countPtr = (volatile unsigned int*)&list.count;
            *countPtr = *countPtr - *countPtr;
            activatedCleanup.release();
            appFreeUni__FPv((void*)list.data);
        }
        return 0;
    }
    for (unsigned int i = 0; i < list.count; ++i, byteOffset += sizeof(int)) {
        // Preserve the byte cursor: dividing it for typed indexing changes register allocation.
        int index = *(int*)((char*)list.data + byteOffset);
        if (index < kFirstActiveSlotElement || index > kLastActiveSlotElement)
            continue;
        AsCLoadDataMenuInEntry(self)->selectionDelayFrames = kSelectionFeedbackFrames;
        Layout_SetElementValueState(self->layout, index, 2);
        Layout_SetElementValueState(self->layout, index + kVisibleSlotCount, 2);
        Layout_SetElementPlayerFlag(self->layout, index, 0, 5);
        Layout_SetElementPlayerFlag(self->layout, index + kVisibleSlotCount, 0, 5);
        if (*(int*)((char*)list.data + byteOffset) == kLastActiveSlotElement &&
            !AsCLoadDataMenuInEntry(self)->memoryRecordLoaded) {
            if (!self->memoryLoadMenu)
                self->memoryLoadMenu = CPadMemoryMenuLoad::Create(self->memoryLoadContext);
            if (list.data) {
                volatile unsigned int* countPtr = (volatile unsigned int*)&list.count;
                *countPtr = *countPtr - *countPtr;
                activatedCleanup.release();
                appFreeUni__FPv((void*)list.data);
            }
            return 1;
        }
        AsCLoadDataMenuInEntry(self)->selectedItem = *(int*)((char*)list.data + byteOffset) - kActiveSlotElementBase;
        break;
    }
    if (list.data) {
        volatile unsigned int* countPtr = (volatile unsigned int*)&list.count;
        *countPtr = *countPtr - *countPtr;
        activatedCleanup.release();
        appFreeUni__FPv((void*)list.data);
    }
    return 1;
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

struct BpNames {
    int values[9];
};
#pragma opt_lifetimes off
// Start menu/caption scripts and hide their detail and slot groups before population.
int CLoadDataMenuInEntry::InitBpGroups() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    int* base = (int*)s_saveLoadEntryGroupTables;
    LoadEntryMenuView* obj = self;
    BpObject_StartScriptByName(obj->bpObject, ((SaveLoadEntryStringPool*)saveLoadEntryStrings)->menuScript, 1);
    int* p = base + 0;
    unsigned int i = 0;
    do {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(obj->bpObject, *p), 0);
        ++i;
        ++p;
    } while (i < 15);
    p = (int*)((char*)(base + 16));
    i = 0;
    do {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(obj->bpObject, *p), 0);
        ++i;
        ++p;
    } while (i < 8);
    p = (int*)((char*)(base + 24));
    i = 0;
    do {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(obj->bpObject, *p), 0);
        ++i;
        ++p;
    } while (i < kVisibleSlotCount);
    i = 0;
    p = &saveLoadDetailGroupIds;
    do {
        UIGroup_SetEnabled(UIGroup_GetOrCreateById(obj->bpObject, *p), 0);
        ++i;
        ++p;
    } while (i < 2);
    BpNames* _np = (BpNames*)((int*)((char*)(base + 99)));
    BpNames names = *_np;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_LoadDataEntry);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bp = IdMap_Lookup(Sing_805DC5F0(), (int)(unsigned int)obj->captionBpEntryId);
    BpObject_StartScriptByName(bp, ((SaveLoadEntryStringPool*)saveLoadEntryStrings)->captionScript, 1);
    for (i = 0; i < 9; ++i)
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(bp, (char*)names.values[i]), 0);
    return 1;
}
#pragma opt_lifetimes reset

struct RM {
    int first;
};
#pragma use_lmw_stmw on
// Allocate five cached records and create the entry layout, BP resources, detail text, and texture archive.
CLoadDataMenuInEntry::CLoadDataMenuInEntry(int arg) {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    // Phase 1: initialize five record caches, sentinels and owned-object slots.
    // arg is a borrowed context later passed to CPadMemoryMenuLoad::Create.
    char* base = s_bpCtrlDtorRegistration_LoadDataEntry;
    MPBase1_Ctor(self);
    *(void**)self = __vt__20CLoadDataMenuInEntry;
    void* layout = __nw__FUl(0x5c);
    if (layout)
        Layout_Init(layout, 5);
    self->layout = layout;
    self->bpObject = 0;
    self->bpEntryId = -1;
    AsCLoadDataMenuInEntry(self)->selectionComplete = 0;
    CachedSaveSlot* end;
    CachedSaveSlot* p;
    p = self->entries;
    end = self->entries + kVisibleSlotCount;
    do {
        SaveData_InitPlayerRecord(p);
        ++p;
    } while (p < end);
    void* cached = GetInputManagerSlot();
    int neg = -1;
    self->textFactory = 0;
    AsCLoadDataMenuInEntry(self)->selectedItem = neg;
    AsCLoadDataMenuInEntry(self)->selectionDelayFrames = neg;
    self->slotDetailsText = 0;
    self->playTimeTextTable = 0;
    self->memoryLoadMenu = 0;
    self->memoryLoadContext = (void*)arg;
    AsCLoadDataMenuInEntry(self)->memoryRecordLoaded = 0;
    self->textureResource = 0;
    self->savedScriptIndex = neg;
    if (!cached) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, (base + 0x18));
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    // Phase 2: bind selected-player input; create main/caption script entries.
    Layout_LoadExtended(self->layout, InputManager_GetSelectedPlayer(GetInputManagerSlot()), saveLoadEntryStrings);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            void* object = s_bpCtrl;
            void (*destructor)(void*, int) = CBpCtrl_dtor;
            void* regmem = base;
            __register_global_object(object, destructor, regmem);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(((SaveLoadEntryStringPool*)saveLoadEntryStrings)->menuBpFile, &self->bpEntryId, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            void* object = s_bpCtrl;
            void (*destructor)(void*, int) = CBpCtrl_dtor;
            void* regmem = base;
            __register_global_object(object, destructor, regmem);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    self->bpObject = IdMap_Lookup(Sing_805DC5F0(), self->bpEntryId);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            void* object = s_bpCtrl;
            void (*destructor)(void*, int) = CBpCtrl_dtor;
            void* regmem = base;
            __register_global_object(object, destructor, regmem);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(((SaveLoadEntryStringPool*)saveLoadEntryStrings)->captionBpFile, &self->captionBpEntryId, 0);
    // Phase 3: create/register detail and play-time draws, then acquire the
    // texture archive under constructor-failure cleanup. Repeated singleton
    // setup above/below ensures borrowed shared services at each use site.
    Memory_Clear(self->entries, 0x28c8);
    self->textFactory = g_chatMsg->textFactory;
    WFixedFont_CreateAndRegisterMsgDraw(self->textFactory, &self->slotDetailsText, 1);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, PointerVector_DestroyStorage, (base + 0x30));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    void* pane = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    void* cached4 = Sing_805DC620();
    self->playTimeTextTable = pane;
    int tmp = 0;
    if (!cached4) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, Allocation_Dtor, (base + 0x48));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* data = ResourceTable_FindEntryByName(
        Sing_805DC620(), saveLoadEntryStrings + (int)&((SaveLoadEntryStringPool*)0)->textureArchive, &tmp);
    void* result = __nw__FUl(0x20);
    if (result) {
        try {
            ((CPackTextureResource*)result)
                ->Init((const char*)(saveLoadEntryStrings + (int)&((SaveLoadEntryStringPool*)0)->textureArchive),
                       (void**)tmp, (unsigned int)data);
        } catch (...) {
            __dl__FPv(result);
            throw;
        }
    }
    self->textureResource = result;
}
#pragma use_lmw_stmw reset

struct CopyWords16 {
    unsigned int values[16];
};
struct CopyWords5 {
    unsigned int values[5];
};
struct CopyBytes24 {
    unsigned char values[24];
};
struct CopyBytes8 {
    unsigned char values[8];
};
struct CopyBytes41 {
    unsigned char first[24]; // preserved byte-copy partition; field semantics are not fully identified
    unsigned char a, b, c, d, e, f;
    unsigned char second[8];
    unsigned char g, h, i;
    // Copy all 41 payload bytes without assigning the following alignment bytes.
    CopyBytes41& operator=(const CopyBytes41& other) {
        *(CopyBytes24*)first = *(const CopyBytes24*)other.first;
        a = other.a;
        b = other.b;
        c = other.c;
        d = other.d;
        e = other.e;
        f = other.f;
        *(CopyBytes8*)second = *(const CopyBytes8*)other.second;
        g = other.g;
        h = other.h;
        i = other.i;
        return *this;
    }
};
struct CopyWords151 {
    unsigned int values[151];
};
struct CopyWords28 {
    unsigned int values[28];
};
struct CopyShorts208 {
    unsigned short values[208];
};
struct CopyShorts414 {
    unsigned short values[414];
};
struct EntryResource {
    virtual ~EntryResource();
};
struct EntryRecord {
    CopyWords151 first;
    CopyWords28 second;
    CopyShorts208 third;
    CopyWords16 words;
    CopyBytes41 bytes;
    CopyWords5 extra;
    CopyShorts414 last;
};
// Same partitions as SelectedSaveRecordView: first/profilePrefix at 0,
// second at 0x25C, third at 0x2CC, words at 0x46C, bytes at 0x4AC,
// extra at 0x4D8, last at 0x4EC. Most payload semantics remain unresolved.

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
// Cache the four NAND slots, initialize visible groups/text, and select the shared load-entry script.
void CLoadDataMenuInEntry::EnterMenu() {
    LoadEntryMenuView* self = (LoadEntryMenuView*)this;
    // Phase 1: drop the load-only archive and copy used NAND records by value.
    // cachedSlotCursor is menu+i*recordBytes; only its entries[0] view is valid.
    if (self->textureResource) {
        delete (EntryResource*)self->textureResource;
        self->textureResource = 0;
    }
    int slotIndex = 0;
    unsigned char* cachedSlotCursor = (unsigned char*)this;
    for (; slotIndex < kNandSlotCount; ++slotIndex, cachedSlotCursor += sizeof(SavePlayerRecord)) {
        if (!GetSaveDataSlot()) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            GetSaveDataSlot() = s_saveData;
        }
        void* src = SaveData_IsSlotUsed(GetSaveDataSlot(), slotIndex);
        if (src) {
            if (!GetSaveDataSlot()) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                GetSaveDataSlot() = s_saveData;
            }
            void* record = SaveData_GetSlotRecord(GetSaveDataSlot(), slotIndex);
            *(EntryRecord*)&((LoadEntryMenuView*)cachedSlotCursor)->entries[0] = *(EntryRecord*)record;
        }
    }
    // Phase 2: populate visibility and place/register the detail/play-time draws.
    ((CLoadDataMenuInEntry*)self)->InitBpGroups();
    ((CLoadDataMenuInEntry*)self)->ResetSlotLayout();
    unsigned char position[12];
    void* pane = UIGroup_GetOrCreateById(self->bpObject, kSlotDetailsGroupId);
    UIGroup_GetPosition(pane, position);
    Text_SetPosition(self->slotDetailsText, position);
    ((CFontNumDraw*)(self->playTimeTextTable))
        ->RegisterEntries(s_saveLoadPlayTimeNumberDescriptors, 3, self->bpObject,
                          ((SaveLoadEntryStringPool*)saveLoadEntryStrings)->numberFont);
    if (!((EntryState*&)Sing_805DC5F8())) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle.entryId = -1;
            s_bpCtrlHandle.scriptIndex = -1;
            __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_80339F44);
            s_bpCtrlHandleInitialized = 1;
        }
        ((EntryState*&)Sing_805DC5F8()) = &s_bpCtrlHandle;
    }
    // Phase 3: save shared script selection, select load-entry script 13 and
    // enter fade-menu behavior. The destructor restores savedScriptIndex.
    self->savedScriptIndex = ((EntryState*&)Sing_805DC5F8())->value();
    if (!((EntryState*&)Sing_805DC5F8())) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle.entryId = -1;
            s_bpCtrlHandle.scriptIndex = -1;
            __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     s_bpCtrlHandleDtorRegistration_80339F44);
            s_bpCtrlHandleInitialized = 1;
        }
        ((EntryState*&)Sing_805DC5F8()) = &s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)(((EntryState*&)Sing_805DC5F8())))->SetScriptIndex(kLoadEntryScriptIndex);
    IFadeMenuBase_StartMenuFadeOut(self);
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
} // extern "C"
