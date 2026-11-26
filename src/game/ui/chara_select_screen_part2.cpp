// Character select screen: joining and clearing player slots, setting up each slot's avatar,
// random and computer choices, and drawing and polling the screen while it runs.
// Also the fade overlay and the small word-vector helpers.

#include "classes/CCharaSelectTransition.h"
#include "classes/CEnemyRenderRecord.h"
// RenderContext record base 0x20D0: name +0x54, flags +0x1E, render object +0xA0.
#define AsCEnemyRenderRecord(p) ((CEnemyRenderRecord*)(p))
#include "classes/CEnemyRenderTable.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/vector.h"
#include "classes/SavePlayerRecord.h"
#include "classes/MWStringStorage.h"

#include "game/singletons.h"
#include "classes/CCharaSelect.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CLoadDataMenuInEntry.h"
#include "classes/CCharacterPreviewRecord.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CBpCtrl.h"

#define AsCCharaSelect(p) ((CCharaSelect*)(p))
// The render context begins with four save records, then four slot vectors and the avatar table.
struct CharaRenderContextStorage {
    SavePlayerRecord saveRecords[4];
    IntVector slotPlayers[4];
    CEnemyRenderTable avatars;
};
struct CharaSelectStrings {
    char archive[17], layout[14], primaryScript[14], noEntryScript[23];
    char vectorLengthError[20], lengthErrorType[55], decideAnimation[20];
    char decideSound[23], clickSound[14], defaultPose[9];
};
enum PlayerSelectionState {
    kSlotEmpty = 0,
    kSlotRandom = 1,
    kSlotChosen = 2
};
// Zero-based player p: human join=42+p, CPU join=46+p; highlights=50+p and
// 54+p respectively. During join the chosen highlight is on; at tick 10 it and
// both join buttons for that slot turn off. Tick 11 wraps the transition counter.
// Character choices 33..40 map to character indices 0..7, load-save=41.
// Other labels: player=58+p, random=62+p; controller c's choice for player p
// is 66+4*p+c (four controller choices per player). Controller and player indices
// differ: the controller's vector back() identifies its current player record.
enum CharaSelectElement {
    kFirstCharacterChoice = 33,
    kLastCharacterChoice = 40,
    kLoadSaveData = 41,
    kFirstHumanJoin = 42,
    kHumanJoinSlot1 = 43,
    kHumanJoinSlot2 = 44,
    kLastHumanJoin = 45,
    kFirstComputerJoin = 46,
    kComputerJoinSlot1 = 47,
    kComputerJoinSlot2 = 48,
    kLastComputerJoin = 49,
    kFirstJoinHighlight = 50,
    kHumanHighlightSlot1 = 51,
    kHumanHighlightSlot2 = 52,
    kHumanHighlightSlot3 = 53,
    kComputerHighlightSlot0 = 54,
    kComputerHighlightSlot1 = 55,
    kComputerHighlightSlot2 = 56,
    kLastJoinHighlight = 57,
    kFirstPlayerLabel = 0x3A,
    kFirstRandomLabel = 0x3E,
    kFirstSlotChoice = 0x42
};
enum {
    kPlayerSlots = 4,
    kCharacterCount = 8,
    kJoinCompleteTick = 10,
    kJoinWrapTick = 11,
    kBackAction = 6,
    kNameChangedFlag = 0x4000,
    kComputerPlayerFlag = 0x8000,
    kLoadMenuBytes = 0x291C,
    kTransitionWords = sizeof(CCharaSelectTransition) / sizeof(int),
    kTransitionActiveWord = (int)&((CCharaSelectTransition*)0)->active / sizeof(int),
    kTransitionTickWord = (int)&((CCharaSelectTransition*)0)->updateTick / sizeof(int),
    kTransitionElementWord = (int)&((CCharaSelectTransition*)0)->activatedElement / sizeof(int)
};
// Preserve the singleton word-reference ABI while naming its role at each access.
#define CHARA_CONTEXT_ADDRESS ((unsigned int&)Sing_805DC648())
#define CHARA_CONTEXT_OFFSET(field) ((int)&((CharaRenderContextStorage*)0)->field)
typedef char CharaContext_slotPlayers_offset[(CHARA_CONTEXT_OFFSET(slotPlayers) == 0x20A0) ? 1 : -1];
typedef char CharaContext_avatars_offset[(CHARA_CONTEXT_OFFSET(avatars) == 0x20D0) ? 1 : -1];
extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u80243434 {
    char* base;
    bool active;
    // Arm cleanup of the partially constructed render aggregate.
    explicit RenderContextConstructionCleanup_u80243434(void* object) : base((char*)object), active(true) {}
    // Release avatar records and vector allocations when initialization throws.
    ~RenderContextConstructionCleanup_u80243434() {
        if (active) {
            CharacterPreviewTable_dtor(base + CHARA_CONTEXT_OFFSET(avatars), -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + CHARA_CONTEXT_OFFSET(slotPlayers) + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Initialize the shared render aggregate with cleanup armed across exceptions.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u80243434 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

void __dl__FPv(void*);

extern signed char s_fadeOverlayInitialized;
extern unsigned char s_fadeOverlay[];
extern unsigned char s_charaSelectUnusedDtorRegistration[];
extern CSharedDoc* g_sharedDoc;
extern signed char s_bpCtrlInitialized;
extern unsigned char s_bpCtrl[];
extern unsigned char s_bpCtrlDtorRegistration_805041C4[];
extern signed char s_renderContextInitialized;
extern unsigned char s_renderContext[];
extern unsigned char s_renderContextRegistrations_805041E8[];
extern unsigned char kCharacterPreviewLoadingIndicatorPositions[];
extern unsigned char charaSelectStrings[];
extern float kHoveredCharacterPreviewScale;
extern float kFloat_0_5_CharaSelect;
extern float kFloat_200_CharaSelect;
extern float kFloat_1_CharaSelect;
extern void* g_soundManager;
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
unsigned int rand(void);
void CharaSelect_UpdateSlotRenderRecord(void*, int, int, int, int, int);
void CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void* RenderContext_GetOrCreate(void);
void SaveData_CopyCurrentRecordToArray(void*, int);
void MPBase1_DrawIfInitialized(void*);
void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void __construct_array(void*, void (*)(void*), void (*)(void*), int, int);
void RenderContext_ResetPlayerRecords(void*);
void __dt__13RenderContextFv(void*);
int strcmp(void*, void*);
unsigned int strlen(const char*);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void Layout_SetActive(void*, int);
void Sound_PlayByName(void*, void*, int, int, int, int, float, float, float);
void* __nw__FUl(unsigned long);
void FadeState_StartFadeIn(void*, int);
void* SharedDoc_GetCurrent();
void LayoutIntVector_PushBack(void*, void*);
int FadeState_GetState(void*);
void dtor_8008A5B4(void*, int);
int Input_IsActionPressed(void*, int, int);
void* LayoutManager_GetInstance();
void* CBpCtrl_GetInstance();
int Vector_GetCount(void*);
void Layout_Update(void*);
int Layout_CollectActivated(void*, void*, void*);
void InputManager_SetSelectedPlayer(void*, int);

struct ObjectListEntry {
    char pad[0xA0];
    void* object;
};
struct ObjectList {
    char pad[0x20D0];
    ObjectListEntry entries[1];
};
struct CharaSelectObjectInterface {
    virtual void Destroy(int x = 0);
    virtual void m1();
    virtual void m2(int x = 0);
};
struct VObjNoArg {
    virtual void s0();
    virtual void s1();
    virtual void s2();
};

#pragma use_lmw_stmw on
// Confirm a slot's character, initialize its preview, and switch to the decision animation.
void CCharaSelect::SetupSlotAvatar(int playerIndex, int controllerSlot, int characterIndex) {
    // Phase 1: initialize the shared aggregate before borrowing any player records.
    // playerIndex addresses save/avatar rows; controllerSlot addresses input ownership.
    unsigned int renderContext = CHARA_CONTEXT_ADDRESS;
    playerStates[playerIndex] = kSlotChosen;
    if (renderContext == 0) {
        if (s_renderContextInitialized == 0) {
            void* base = s_renderContext;
            void* end = (char*)base + CHARA_CONTEXT_OFFSET(slotPlayers);
            void* p = base;
            do {
                SaveData_InitPlayerRecord(p);
                p = (char*)p + sizeof(SavePlayerRecord);
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            void* arr = (char*)base + CHARA_CONTEXT_OFFSET(avatars);
            __construct_array(arr, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord),
                              kPlayerSlots);
            AsCEnemyRenderTable(arr)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        CHARA_CONTEXT_ADDRESS = (unsigned int)s_renderContext;
    }
    // Phase 2: update the chosen render record and place its preview in the slot rectangle.
    int offset = playerIndex * sizeof(CEnemyRenderRecord);
    void* data = (char*)CHARA_CONTEXT_ADDRESS + offset;
    CharaSelect_UpdateSlotRenderRecord(
        this, controllerSlot, characterIndex,
        (AsCEnemyRenderRecord((char*)data + CHARA_CONTEXT_OFFSET(avatars))->flags >> 15) & 1, playerIndex, 0);
    int previewRectangle[4];
    previewRectangle[1] = 0x78;
    float previewScale = kHoveredCharacterPreviewScale;
    previewRectangle[0] = playerIndex * 0x96 + 0x14;
    previewRectangle[2] = playerIndex * 0x96 + 0x14 + 0x96;
    previewRectangle[3] = 0x190;
    void* board = (char*)RenderContext_GetOrCreate() + CHARA_CONTEXT_OFFSET(avatars);
    if (playerIndex >= 0 && AsCEnemyRenderTable(board)->count > playerIndex)
        ((CCharacterPreviewRecord*)((char*)board + offset))->SetupAvatar(previewRectangle, 0, previewScale);
    void* item = (char*)RenderContext_GetOrCreate() + offset;
    void* effect0 = AsCEnemyRenderRecord((char*)item + CHARA_CONTEXT_OFFSET(avatars))->renderObject;
    if (effect0) {
        void* item2 = (char*)RenderContext_GetOrCreate() + offset;
        void* effect = AsCEnemyRenderRecord((char*)item2 + CHARA_CONTEXT_OFFSET(avatars))->renderObject;
        ((CAsyncExhibition*)effect)
            ->SetLoadingIndicatorPosition(
                (const float*)((char*)kCharacterPreviewLoadingIndicatorPositions + playerIndex * 8));
    }
    if (CHARA_CONTEXT_ADDRESS == 0) {
        if (s_renderContextInitialized == 0) {
            void* base = s_renderContext;
            void* end = (char*)base + CHARA_CONTEXT_OFFSET(slotPlayers);
            void* p = base;
            do {
                SaveData_InitPlayerRecord(p);
                p = (char*)p + sizeof(SavePlayerRecord);
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            void* arr = (char*)base + CHARA_CONTEXT_OFFSET(avatars);
            __construct_array(arr, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord),
                              kPlayerSlots);
            AsCEnemyRenderTable(arr)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, __dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        CHARA_CONTEXT_ADDRESS = (unsigned int)s_renderContext;
    }
    // Phase 3: replace the current animation name only when it differs; mark dirty.
    char* animationStrings = (char*)charaSelectStrings;
    void* root = (void*)CHARA_CONTEXT_ADDRESS;
    char* decisionAnimation = animationStrings + ((int)&((CharaSelectStrings*)0)->decideAnimation);
    if (playerIndex >= 0) {
        if (AsCEnemyRenderTable((char*)root + CHARA_CONTEXT_OFFSET(avatars))->count > playerIndex) {
            char* current =
                (char*)AsCEnemyRenderRecord((char*)root + offset + CHARA_CONTEXT_OFFSET(avatars))->animationName;
            unsigned int stringTag = *(unsigned int*)current;
            int usesInlineName = !(stringTag >> 31);
            const char* currentAnimationName = usesInlineName ? current + 1 : *(const char**)(current + 8);
            if (strcmp(decisionAnimation, (void*)currentAnimationName)) {
                int currentNameLength = usesInlineName ? (unsigned char)current[0] & 0x7F : *(int*)(current + 4);
                unsigned int animationNameLength = strlen(decisionAnimation);
                char buf;
                String_ReplaceRange(current, 0, currentNameLength, decisionAnimation,
                                    decisionAnimation + animationNameLength, &buf);
                unsigned short* flags =
                    &AsCEnemyRenderRecord((char*)root + offset + CHARA_CONTEXT_OFFSET(avatars))->flags;
                *flags |= kNameChangedFlag;
            }
        }
    }
    UpdateSlotInputAvailability();
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Clear a slot's choices and preview, then restore its default player save record.
void CCharaSelect::ClearSlot(int index, int slot) {
    int offset;
    int scaled = index * 4;
    playerStates[index] = kSlotEmpty;
    Layout_SetElementEnabled(layout, scaled + kFirstSlotChoice, 0);
    Layout_SetElementEnabled(layout, scaled + kFirstSlotChoice + 1, 0);
    Layout_SetElementEnabled(layout, scaled + kFirstSlotChoice + 2, 0);
    Layout_SetElementEnabled(layout, scaled + kFirstSlotChoice + 3, 0);
    Layout_SetElementEnabled(layout, index + kFirstRandomLabel, 0);
    Layout_SetElementEnabled(layout, index + kFirstPlayerLabel, 0);
    Layout_SetElementPlayerFlagsAndState(layout, index + kFirstHumanJoin, 1);
    unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
    if (mode - 6 <= 1)
        Layout_SetElementPlayerFlagsAndState(layout, index + kFirstComputerJoin, 0);
    else
        Layout_SetElementPlayerFlagsAndState(layout, index + kFirstComputerJoin, 1);

    if (CHARA_CONTEXT_ADDRESS == 0) {
        if (s_renderContextInitialized == 0) {
            void* base = s_renderContext;
            void* end = (char*)base + CHARA_CONTEXT_OFFSET(slotPlayers);
            void* p = base;
            do {
                SaveData_InitPlayerRecord(p);
                p = (char*)p + sizeof(SavePlayerRecord);
            } while (p < end);
            ((IntVectorArray*)(end))->__ct();
            void* arr = (char*)base + CHARA_CONTEXT_OFFSET(avatars);
            __construct_array(arr, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor, sizeof(CEnemyRenderRecord),
                              kPlayerSlots);
            AsCEnemyRenderTable(arr)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(base, __dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        CHARA_CONTEXT_ADDRESS = (unsigned int)s_renderContext;
    }
    // Phase 2: destroy only this slot's preview object; re-read the singleton at
    // original boundaries before clearing the handle and restoring save data.
    offset = index * sizeof(CEnemyRenderRecord);
    void* root = (void*)CHARA_CONTEXT_ADDRESS;
    void* current = (char*)root + offset;
    if ((int)AsCEnemyRenderRecord((char*)current + CHARA_CONTEXT_OFFSET(avatars))->renderObject != 0) {
        if (CHARA_CONTEXT_ADDRESS == 0) {
            if (s_renderContextInitialized == 0) {
                void* base = s_renderContext;
                void* end = (char*)base + CHARA_CONTEXT_OFFSET(slotPlayers);
                void* p = base;
                do {
                    SaveData_InitPlayerRecord(p);
                    p = (char*)p + sizeof(SavePlayerRecord);
                } while (p < end);
                ((IntVectorArray*)(end))->__ct();
                void* arr = (char*)base + CHARA_CONTEXT_OFFSET(avatars);
                __construct_array(arr, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor,
                                  sizeof(CEnemyRenderRecord), kPlayerSlots);
                AsCEnemyRenderTable(arr)->count = 0;
                InitializeNewRenderContext(base);
                __register_global_object(base, __dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
                s_renderContextInitialized = 1;
            }
            CHARA_CONTEXT_ADDRESS = (unsigned int)s_renderContext;
        }
        root = (void*)CHARA_CONTEXT_ADDRESS;
        void* object = AsCEnemyRenderRecord((char*)root + offset + CHARA_CONTEXT_OFFSET(avatars))->renderObject;
        if (object) {
            ((struct CharaSelectObjectInterface*)object)->Destroy(1); // First virtual slot is the deleting destructor.
        }
        if (CHARA_CONTEXT_ADDRESS == 0) {
            if (s_renderContextInitialized == 0) {
                void* base = s_renderContext;
                void* end = (char*)base + CHARA_CONTEXT_OFFSET(slotPlayers);
                void* p = base;
                do {
                    SaveData_InitPlayerRecord(p);
                    p = (char*)p + sizeof(SavePlayerRecord);
                } while (p < end);
                ((IntVectorArray*)(end))->__ct();
                void* arr = (char*)base + CHARA_CONTEXT_OFFSET(avatars);
                __construct_array(arr, PartyItemRecord_InitDefaults, PlayerRenderRecord_dtor,
                                  sizeof(CEnemyRenderRecord), kPlayerSlots);
                AsCEnemyRenderTable(arr)->count = 0;
                InitializeNewRenderContext(base);
                __register_global_object(base, __dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
                s_renderContextInitialized = 1;
            }
            CHARA_CONTEXT_ADDRESS = (unsigned int)s_renderContext;
        }
        root = (void*)CHARA_CONTEXT_ADDRESS;
        AsCEnemyRenderRecord((char*)root + offset + CHARA_CONTEXT_OFFSET(avatars))->renderObject = 0;
    }
    SaveData_CopyCurrentRecordToArray(RenderContext_GetOrCreate(), index);
    UpdateSlotInputAvailability();
}
#pragma use_lmw_stmw reset

// Draw both character-selection scripts, the layout, and any active load-data modal.
void CCharaSelect::Draw() {
    unsigned int init = ((unsigned int&)Sing_805DC5F0());
    if (init == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    // Phase 1: invoke and draw the two script entries with lazy BP access intact.
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup((void*)((unsigned int&)Sing_805DC5F0()), primaryBpEntryId), 0);
    if (((unsigned int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup((void*)((unsigned int&)Sing_805DC5F0()), primaryBpEntryId));
    if (((unsigned int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup((void*)((unsigned int&)Sing_805DC5F0()), secondaryBpEntryId), 0);
    if (((unsigned int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup((void*)((unsigned int&)Sing_805DC5F0()), secondaryBpEntryId));
    // Phase 2: draw the layout, live avatar previews, then the owned save-data modal.
    Layout_DrawWithOpacity(layout, kFloat_1_CharaSelect);
    char* list = (char*)RenderContext_GetOrCreate() + CHARA_CONTEXT_OFFSET(avatars);
    int i = 0;
    char* p = list;
    while (i < AsCEnemyRenderTable(list)->count) {
        void* object = AsCEnemyRenderRecord(p)->renderObject;
        if (object != 0) {
            ((VObjNoArg*)object)->s2();
        }
        p += 0xA4;
        i++;
    }
    void* tail = loadDataMenu;
    if (tail != 0)
        MPBase1_DrawIfInitialized(tail);
}

// Reset slot controls and enable the join options supported by the current game mode.
void CCharaSelect::ResetPlayerSlot(int index) {
    unsigned int offset = index * 4;
    (*(unsigned int*)&playerStates[index]) = 0;
    Layout_SetElementEnabled(layout, offset + kFirstSlotChoice, 0);
    Layout_SetElementEnabled(layout, offset + kFirstSlotChoice + 1, 0);
    Layout_SetElementEnabled(layout, offset + kFirstSlotChoice + 2, 0);
    Layout_SetElementEnabled(layout, offset + kFirstSlotChoice + 3, 0);
    Layout_SetElementEnabled(layout, index + kFirstRandomLabel, 0);
    Layout_SetElementEnabled(layout, index + kFirstPlayerLabel, 0);
    Layout_SetElementPlayerFlagsAndState(layout, index + kFirstHumanJoin, 1);
    unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
    if (mode == 6 || mode == 7)
        Layout_SetElementPlayerFlagsAndState(layout, index + kFirstComputerJoin, 0);
    else
        Layout_SetElementPlayerFlagsAndState(layout, index + kFirstComputerJoin, 1);
}

#pragma use_lmw_stmw on
// Choose a random preview character and configure human/computer slot controls.
void CCharaSelect::SetRandomSlot(int playerIndex, int controllerSlot, int computerPlayer, int updatePreview) {
    unsigned int offset = playerIndex * 4;
    playerStates[playerIndex] = kSlotRandom;
    int randomValue = (int)rand();
    int characterIndex = randomValue % kCharacterCount;
    if (updatePreview != 0)
        CharaSelect_UpdateSlotRenderRecord(this, controllerSlot, characterIndex, computerPlayer, playerIndex,
                                           updatePreview);
    int i = 0;
    do {
        if (i == controllerSlot) {
            Layout_SetElementPlayerFlagsAndState(layout, offset + kFirstSlotChoice, 1);
            Layout_SetElementEnabled(layout, playerIndex, 1);
        } else {
            Layout_SetElementEnabled(layout, offset + kFirstSlotChoice, 0);
        }
        if (computerPlayer != 0)
            Layout_SetElementEnabled(layout, offset + kFirstSlotChoice, 0);
        i++;
        offset++;
    } while (i < kPlayerSlots);
    if (computerPlayer != 0) {
        Layout_SetElementEnabled(layout, playerIndex + kFirstPlayerLabel, 1);
        Layout_SetElementEnabled(layout, playerIndex + kFirstRandomLabel, 0);
    } else {
        Layout_SetElementEnabled(layout, playerIndex + kFirstRandomLabel, 1);
        Layout_SetElementEnabled(layout, playerIndex + kFirstPlayerLabel, 0);
    }
    UpdateSlotInputAvailability();
}
#pragma use_lmw_stmw reset

struct CharaWordVector {
    unsigned int* data;
    unsigned int count;
    unsigned int capacity;
};

struct HasWordAt28 {
    char padding[0x28];
    unsigned int value;
};

// Initialize an empty unsigned-word vector header.
void CharaSelectWordVector_InitializeEmpty(CharaWordVector*);
// Return the last element of a nonempty unsigned-word vector.
unsigned int* CharaSelectWordVector_GetBack(CharaWordVector*);
// Report the confirmation result at CCharaSelect +0x28 (written by its update routine).
// Return the indexed unsigned-word element without bounds checking.
unsigned int* CharaSelectWordVector_GetElement(CharaWordVector*, unsigned int);
// Return the indexed event-code element without bounds checking.
unsigned int* WordVector_GetElementAddress(CharaWordVector*, unsigned int);
// Remove the last vector element by decrementing its count.
void CharaSelectWordVector_PopBack(CharaWordVector*);

// Lazily construct and register the shared fade overlay.
void* FadeOverlay_GetInstance();

int MPBase1_Process(void*);
void* RenderContext_GetInstance();
void FadeState_StartFadeOut(void*, int);
void RenderContext_UpdateCharacterPreviews(float);
void BpObject_SetEnabled(void*, int);

// Finish one slot's join animation: hide its selected highlight and both join buttons.
static inline void HideCompletedJoin(void* self, int highlightElement, int humanElement, int computerElement) {
    Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&AsCCharaSelect(self)->layout), highlightElement, 0);
    Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&AsCCharaSelect(self)->layout), humanElement, 0);
    Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&AsCCharaSelect(self)->layout), computerElement, 0);
}

// Update save-data modal handling, join animations, character input, cancellation, and completion.
// Returns 1 when navigation may finish; selectionConfirmed distinguishes confirm
// from cancel. Otherwise returns 0. self owns loadDataMenu and transition storage;
// render records/save records are borrowed singleton storage. deltaTime is forwarded
// to preview animation; the join counter below advances in update ticks, not seconds.
int CCharaSelect::UpdateAndPollFinished(float deltaTime) {
    // Keep the transition word view below: direct struct indexing changes CodeWarrior scheduling.
    // Phase 1: modal owns input until completion; copy its record, restore layout
    // and fade state, then defer normal input until a later update.
    if ((*(int*)&loadDataMenu) != 0) {
        if (MPBase1_Process((void*)(*(int*)&loadDataMenu)) != 0) {
            int item = (int)*CharaSelectWordVector_GetBack((CharaWordVector*)((char*)RenderContext_GetInstance() +
                                                                              selectedSlot * 12 +
                                                                              CHARA_CONTEXT_OFFSET(slotPlayers)));
            char defaultPlayerRecord[sizeof(SavePlayerRecord)];
            SaveData_InitPlayerRecord(&defaultPlayerRecord);
            if (((CLoadDataMenuInEntry*)((*(int*)&loadDataMenu)))
                    ->CopySelectedRecord((char*)RenderContext_GetInstance() + item * sizeof(SavePlayerRecord)) != 0) {
                CharaSelect_UpdateSlotRenderRecord(
                    this, selectedSlot,
                    (int)AsSavePlayerRecord((char*)RenderContext_GetInstance() + item * sizeof(SavePlayerRecord))
                        ->characterIndex,
                    0, item, 0);
                UpdateSlotInputAvailability();
            }
            InputManager_SetSelectedPlayer(LayoutManager_GetInstance(), 0);
            Layout_SetActive((void*)(*(int*)&layout), 1);
            void* object = (void*)(*(int*)&loadDataMenu);
            if (object != 0) {
                if (object != 0) {
                    ((struct CharaSelectObjectInterface*)object)->Destroy(1);
                }
                (*(int*)&loadDataMenu) = 0;
            }
            ((CBpCtrlHandle*)(CBpCtrlHandle::GetInstance()))->SetScriptIndex(14);
            FadeState_StartFadeOut(FadeOverlay_GetInstance(), 15);
            inputReady = 0;
        }
        return 0;
    }
    // Phase 2: advance previews and per-slot joins before collecting input.
    RenderContext_UpdateCharacterPreviews(deltaTime);
    IntVector activatedCodes;
    CharaWordVector activatedSlots;
    CharaSelectWordVector_InitializeEmpty(&activatedSlots);
    BpObject_SetEnabled(IdMap_Lookup(CBpCtrl_GetInstance(), (void*)(*(int*)&primaryBpEntryId)), 1);
    BpObject_SetEnabled(IdMap_Lookup(CBpCtrl_GetInstance(), (void*)(*(int*)&secondaryBpEntryId)), 1);
    // Join storage schema: active word 0, update tick word 1, activated element
    // word 2; named indexes derive from CCharaSelectTransition. The eight cases
    // retain the human/CPU highlight map above, and completion hides both join
    // buttons for that player through HideCompletedJoin. Keep the word view to
    // preserve CodeWarrior scheduling across the active/tick/element accesses.
    if (((CCharaSelectTransition*)playerTransitions)[0].active ||
        ((CCharaSelectTransition*)playerTransitions)[1].active ||
        ((CCharaSelectTransition*)playerTransitions)[2].active ||
        ((CCharaSelectTransition*)playerTransitions)[3].active) {
        for (int i = 0; i < kPlayerSlots; i++) {
            if (((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionTickWord] == kJoinWrapTick)
                ((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionTickWord] = 0;
        }
        for (int i = 0; i < kPlayerSlots; i++) {
            switch (((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionElementWord]) {
            case kFirstHumanJoin:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kFirstJoinHighlight, 1);
                break;
            case kHumanJoinSlot1:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kHumanHighlightSlot1, 1);
                break;
            case kHumanJoinSlot2:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kHumanHighlightSlot2, 1);
                break;
            case kLastHumanJoin:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kHumanHighlightSlot3, 1);
                break;
            case kFirstComputerJoin:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kComputerHighlightSlot0, 1);
                break;
            case kComputerJoinSlot1:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kComputerHighlightSlot1, 1);
                break;
            case kComputerJoinSlot2:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kComputerHighlightSlot2, 1);
                break;
            case kLastComputerJoin:
                Layout_SetElementPlayerFlagsAndState((void*)(*(int*)&layout), kLastJoinHighlight, 1);
                break;
            }
            if (((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionTickWord] == kJoinCompleteTick) {
                ((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionActiveWord] = 0;
                ((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionTickWord] = 0;
                switch (((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionElementWord]) {
                case kFirstHumanJoin:
                    HideCompletedJoin(this, kFirstJoinHighlight, kFirstHumanJoin, kFirstComputerJoin);
                    break;
                case kHumanJoinSlot1:
                    HideCompletedJoin(this, kHumanHighlightSlot1, kHumanJoinSlot1, kComputerJoinSlot1);
                    break;
                case kHumanJoinSlot2:
                    HideCompletedJoin(this, kHumanHighlightSlot2, kHumanJoinSlot2, kComputerJoinSlot2);
                    break;
                case kLastHumanJoin:
                    HideCompletedJoin(this, kHumanHighlightSlot3, kLastHumanJoin, kLastComputerJoin);
                    break;
                case kFirstComputerJoin:
                    HideCompletedJoin(this, kComputerHighlightSlot0, kFirstHumanJoin, kFirstComputerJoin);
                    break;
                case kComputerJoinSlot1:
                    HideCompletedJoin(this, kComputerHighlightSlot1, kHumanJoinSlot1, kComputerJoinSlot1);
                    break;
                case kComputerJoinSlot2:
                    HideCompletedJoin(this, kComputerHighlightSlot2, kHumanJoinSlot2, kComputerJoinSlot2);
                    break;
                case kLastComputerJoin:
                    HideCompletedJoin(this, kLastJoinHighlight, kLastHumanJoin, kLastComputerJoin);
                    break;
                }
                ((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionElementWord] = 0;
            }
            if (((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionActiveWord] != 0)
                ((*(int**)&playerTransitions))[i * kTransitionWords + kTransitionTickWord]++;
        }
    }
    // Phase 3: suppress input during fade and for one ready-transition update.
    void* fadeOverlay = (void*)FadeOverlay_GetInstance();
    if (FadeState_GetState(fadeOverlay) == 3) {
        dtor_8008A5B4(&activatedSlots, -1);

        return 0;
    }
    if (inputReady == 0) {
        inputReady = 1;
        dtor_8008A5B4(&activatedSlots, -1);

        return 0;
    }
    // Phase 4: collect parallel arrays: event codes and controller-slot indices.
    // A controller slot's vector back() is its current player-record index.
    Layout_Update((void*)(*(int*)&layout));
    if (Layout_CollectActivated((void*)(*(int*)&layout), &activatedCodes, &activatedSlots) != 0) {
        for (unsigned int i = 0; i < (unsigned int)Vector_GetCount(&activatedCodes); i++) {
            int elem = (int)*CharaSelectWordVector_GetElement(&activatedSlots, i);
            // Activated code/slot arrays must stay parallel throughout this dispatch.
            switch ((int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i)) {
            case kFirstCharacterChoice:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case kLastCharacterChoice: {
                int off = elem * 12;
                if (Vector_GetCount((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off) !=
                    0) {
                    int found = (int)*CharaSelectWordVector_GetBack(
                        (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) +
                                           off));
                    if (playerStates[found] == kSlotRandom) {
                        Sound_PlayByName(g_soundManager,
                                         &charaSelectStrings[(int)&((CharaSelectStrings*)0)->decideSound], 0, 0, 0, 0,
                                         kFloat_0_5_CharaSelect, kFloat_200_CharaSelect, kFloat_1_CharaSelect);
                        int found2 = (int)*CharaSelectWordVector_GetBack(
                            (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) +
                                               off));
                        SetupSlotAvatar(found2, elem,
                                        (int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i) -
                                            kFirstCharacterChoice);
                    }
                }
                break;
            }
            case kLoadSaveData: {
                Sound_PlayByName(g_soundManager, &charaSelectStrings[(int)&((CharaSelectStrings*)0)->clickSound], 0, 0,
                                 0, 0, kFloat_0_5_CharaSelect, kFloat_200_CharaSelect, kFloat_1_CharaSelect);
                if ((*(int*)&loadDataMenu) == 0) {
                    selectedSlot = elem;
                    void* p = __nw__FUl(kLoadMenuBytes);
                    if (p != 0) {
                        try {
                            ((CLoadDataMenuInEntry*)p)->__ct(elem);
                        } catch (...) {
                            __dl__FPv(p);
                            throw;
                        }
                    }
                    (*(int*)&loadDataMenu) = (int)p;
                    InputManager_SetSelectedPlayer(LayoutManager_GetInstance(), elem);
                    Layout_SetActive((void*)(*(int*)&layout), 0);
                    FadeState_StartFadeIn(FadeOverlay_GetInstance(), 0);
                }
                dtor_8008A5B4(&activatedSlots, -1);

                return 0;
            }
            case kFirstHumanJoin:
            case kHumanJoinSlot1:
            case kHumanJoinSlot2:
            case kLastHumanJoin: {
                Sound_PlayByName(g_soundManager, &charaSelectStrings[(int)&((CharaSelectStrings*)0)->clickSound], 0, 0,
                                 0, 0, kFloat_0_5_CharaSelect, kFloat_200_CharaSelect, kFloat_1_CharaSelect);
                int off = elem * 12;
                ((CCharaSelectTransition*)((char*)playerTransitions + off))->active = 1;
                int code2 = (int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i);
                ((CCharaSelectTransition*)((char*)playerTransitions + off))->activatedElement = code2;
                if (Vector_GetCount((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off) !=
                    0) {
                    int found = (int)*CharaSelectWordVector_GetBack(
                        (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) +
                                           off));
                    if (playerStates[found] != kSlotChosen)
                        continue;
                }
                int local = (int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i) - kFirstHumanJoin;
                if (playerStates[local] != kSlotEmpty)
                    continue;
                LayoutIntVector_PushBack((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off,
                                         &local);
                SetRandomSlot(local, elem, 0, 1);
                break;
            }
            case kFirstComputerJoin:
            case kComputerJoinSlot1:
            case kComputerJoinSlot2:
            case kLastComputerJoin: {
                Sound_PlayByName(g_soundManager, &charaSelectStrings[(int)&((CharaSelectStrings*)0)->clickSound], 0, 0,
                                 0, 0, kFloat_0_5_CharaSelect, kFloat_200_CharaSelect, kFloat_1_CharaSelect);
                int off = elem * 12;
                ((CCharaSelectTransition*)((char*)playerTransitions + off))->active = 1;
                int code2 = (int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i);
                ((CCharaSelectTransition*)((char*)playerTransitions + off))->activatedElement = code2;
                if (Vector_GetCount((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off) !=
                    0) {
                    int found = (int)*CharaSelectWordVector_GetBack(
                        (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) +
                                           off));
                    if (playerStates[found] != kSlotChosen)
                        continue;
                }
                int local =
                    (int)*WordVector_GetElementAddress((CharaWordVector*)&activatedCodes, i) - kFirstComputerJoin;
                if (playerStates[local] != kSlotEmpty)
                    continue;
                LayoutIntVector_PushBack((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off,
                                         &local);
                SetRandomSlot(local, elem, 1, 1);
                break;
            }
            default:
                break;
            }
        }
    }
    // Phase 5: back moves chosen -> random -> removed; joins block removal.
    int noPlayersJoined = 1;
    for (int i = 0; i < kPlayerSlots; i++) {
        int off = i * 12;
        if (Vector_GetCount((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off) == 0)
            continue;
        noPlayersJoined = 0;
        int elem = (int)*CharaSelectWordVector_GetBack(
            (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off));
        if (!Input_IsActionPressed(LayoutManager_GetInstance(), kBackAction, i))
            continue;
        int state = playerStates[elem];
        switch (state) {
        case kSlotChosen: {
            unsigned int bit = (AsCEnemyRenderRecord((char*)RenderContext_GetInstance() +
                                                     elem * sizeof(CEnemyRenderRecord) + CHARA_CONTEXT_OFFSET(avatars))
                                    ->flags >>
                                15) &
                               1;
            SetRandomSlot(elem, i, bit, 1);
            ((CCharacterPreviewTable*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(avatars)))
                ->SetAnimationName(elem, (const char*)&charaSelectStrings[(int)&((CharaSelectStrings*)0)->defaultPose]);
            break;
        }
        case kSlotRandom:
            if (((CCharaSelectTransition*)((char*)playerTransitions + off))->active == 0) {
                CharaSelectWordVector_PopBack(
                    (CharaWordVector*)((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(slotPlayers) + off));
                ClearSlot(elem, i);
            }
            break;
        }
    }
    // Phase 6: distinguish all-chosen confirmation from back/cancel. The sliding
    // p view reads consecutive playerStates words; it does not iterate whole objects.
    int* p = (int*)this;
    int allPlayersChosen = 1;
    for (int i = 0; i < AsCEnemyRenderTable((char*)RenderContext_GetInstance() + CHARA_CONTEXT_OFFSET(avatars))->count;
         p++, i++) {
        if (AsCCharaSelect(p)->playerState != kSlotChosen)
            allPlayersChosen = 0;
    }
    int backPressed = 0;
    for (int i = 0; i < kPlayerSlots; i++) {
        backPressed |= Input_IsActionPressed(LayoutManager_GetInstance(), kBackAction, i);
    }
    int result = 0;
    if (AsCSharedDoc(SharedDoc_GetCurrent())->gameMode == 6 && backPressed) {
        selectionConfirmed = 0;
        result = 1;
    }
    if (allPlayersChosen) {
        selectionConfirmed = 1;
        result = 1;
    } else if (noPlayersJoined && backPressed) {
        selectionConfirmed = 0;
        result = 1;
    }
    if (result != 0) {
        if (((CBpCtrl*)CBpCtrl_GetInstance())->PollAllEntriesLoaded() != 0) {
            BpObject_SetEnabled(IdMap_Lookup(CBpCtrl_GetInstance(), (void*)(*(int*)&primaryBpEntryId)), 0);
            BpObject_SetEnabled(IdMap_Lookup(CBpCtrl_GetInstance(), (void*)(*(int*)&secondaryBpEntryId)), 0);
        }
    }
    UpdateHoveredAvatars();
    dtor_8008A5B4(&activatedSlots, -1);

    return result;
}

// Lazily construct and register the shared fade overlay.
void* FadeOverlay_GetInstance() {
    if (((unsigned int&)Sing_805DC608()) == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_charaSelectUnusedDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
    }
    return (void*)((unsigned int&)Sing_805DC608());
}

// Initialize an empty unsigned-word vector.
void CharaSelectWordVector_InitializeEmpty(CharaWordVector* self) {
    self->data = 0;
    self->count = 0;
    self->capacity = 0;
}

// Return the last element of a nonempty unsigned-word vector.
unsigned int* CharaSelectWordVector_GetBack(CharaWordVector* self) {
    return self->data + (self->count - 1);
}

// Return the selection outcome written by CCharaSelect::UpdateAndPollFinished at +0x28.
// All-chosen writes 1, cancellation writes 0; the title flow uses it for navigation.
int CCharaSelect::IsSelectionConfirmed() {
    HasWordAt28* self = (HasWordAt28*)this;
    return self->value != 0;
}

// Return the indexed unsigned-word element without bounds checking.
unsigned int* CharaSelectWordVector_GetElement(CharaWordVector* self, unsigned int index) {
    return self->data + index;
}

// Return the indexed event-code address; the caller must keep index below count.
unsigned int* WordVector_GetElementAddress(CharaWordVector* self, unsigned int index) {
    return self->data + index;
}

// Pop the last vector element by reducing the count.
void CharaSelectWordVector_PopBack(CharaWordVector* self) {
    self->count--;
}

} // extern "C"
