// Party Goods selection UI construction (CPartyItemSelect, 0x80088DE0).
// Entry map for a layout/script mod:
// - selectionLayout: partygoods_select.spd at partyItemSelectStrings+0, with
//   57 records in s_partyItemSelectionLayoutElements;
//   item n [0,12) uses button 8+n, image 20+n, highlight 32+n, image 44+n;
//   continue is 56. Keep these indices together with UpdateSelectionAndDetail in party_item_select.cpp.
// - selectionBpObject: partygoods_select.b2d at partyItemSelectStrings+0x16;
//   playerCountBpObject: partygoods_noentry.b2d at +0x2c. These animate the UI;
//   layouts describe element placement, and BP entry IDs own script registrations.
// - detailLayout: common_info.spd at +0x43, three records in
//   s_partyItemDetailLayoutElements (1 explanation, 2 name; role of 0 unresolved).
//   detailBpObject uses common_info.b2d at +0x53; open/close script selection and
//   text styles are in party_item_select.cpp's PartyItemSelectStringPool/RefreshDetailText.
// - feedback duration: PartyItemActivation::feedbackFramesRemaining counts UI
//   updates; its initial value here is zero, its four-frame trigger is in UpdateSelectionAndDetail.
// Shared layouts/strings are external data; this unit does not edit those assets.
#include "classes/CPartyItemSelect.h"
#include "classes/PartyItemSelection.h"
#include "classes/PartyItemActivation.h"
#include "classes/SavePlayerRecord.h"
extern "C" void CharacterPreviewTable_dtor(void*, int);
#define GAME_VECTOR_IMPLEMENTATION
#include "game/vector.h"

#include "game/singletons.h"
#include "classes/CBpCtrl.h"
// This font slot lies in the existing class view padding at 0x3c.
struct SelectorFontSlot {
    char padBeforeFont[0x3c];
    void* font;
};
struct ChatFontSlot {
    char padBeforeFont[0x10];
    void* font;
};
static const unsigned int kRenderVectorsOffset = (unsigned int)&((RenderContext*)0)->vectors;
static const unsigned int kRenderSelectionsOffset = (unsigned int)&((RenderContext*)0)->players;
static const int kMaxPlayers = 4;
static const unsigned int kLayoutObjectBytes = 0x5c;
static const int kLayoutInitMode = 5;
static const int kSelectionElementCount = 0x39;
static const int kDetailElementCount = 3;
static const unsigned int kSelectionScriptOffset = 0x16;   // partygoods_select.b2d
static const unsigned int kPlayerCountScriptOffset = 0x2c; // partygoods_noentry.b2d
static const unsigned int kDetailLayoutOffset = 0x43;      // common_info.spd
static const unsigned int kDetailScriptOffset = 0x53;      // common_info.b2d

extern "C" {

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void MPBase1_Ctor(void*);
void Layout_Init(void*, int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void Layout_SetActive(void*, int);
void SaveData_InitPlayerRecord(void*);
void __construct_array(void*, void*, void*, int, int);
void RenderContext_ResetPlayerRecords(void*);
void __register_global_object(void*, void*, void*);
void CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, int);
void* __nwa__FUl(int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void* __nw__FUl(unsigned int);
void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void __dt__13RenderContextFv(void*);
extern char __vt__16CPartyItemSelect[];
extern char s_renderContext[];
extern char s_bpCtrl[];
extern char s_partyItemSelectRenderContextRegistration[];
extern char s_bpCtrlRegistration_804EC728[];
extern const char s_partyItemSelectionLayoutElements[];
extern const char partyItemSelectStrings[];
extern const char s_partyItemDetailLayoutElements[];
extern signed char s_renderContextInitialized;
extern signed char s_bpCtrlInitialized;
extern void* g_chatMsg;

// Construct/register the shared BP controller once; the selector borrows it.
static inline void PartySelector_RegisterBpController() {
    if (s_bpCtrlInitialized == 0) {
        ((CBpCtrl*)s_bpCtrl)->__ct();
        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_804EC728);
        s_bpCtrlInitialized = 1;
    }
    ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
}

// Build both selector layouts, controller entries, per-player state, and detail text.
// Layouts, player arrays and text resources belong to the selector; shared render
// context/controller/font and resolved BP-object pointers are borrowed.
// self is required storage for CPartyItemSelect, not an already-live selector;
// return self after initialization. No success/failure status is returned.
// Allocation/initialization failures are not converted to a null return: layout
// initialization is called even if allocation returned null. Callers require a
// working allocator, chat/font singleton and valid player count [0,4].
// Rollback is local: if RenderContext_ResetPlayerRecords throws, destroy its
// player table then integer vectors and rethrow, before publishing registration.
// There is no enclosing transaction for selector layouts/arrays/BP entries.
// Normal teardown is CPartyItemSelect::Destroy in party_item_select.cpp: release displays,
// layouts, entries, activation array and text resources; flag>0 also frees self.
// Shared controller/render context remain globally registered until shutdown.
// Singleton first-use publication is unsynchronized; use on the UI thread.
CPartyItemSelect::CPartyItemSelect() {
    CPartyItemSelect* selector = this;
    MPBase1_Ctor(this);
    selector->selectionComplete = 0;
    selector->vtbl = __vt__16CPartyItemSelect;
    selector->selectionLayout = 0;
    selector->selectionBpEntryId = -1;
    selector->selectionBpObject = 0;
    selector->playerActivations = 0;
    selector->selectedItemDisplays = 0;
    selector->playerCountBpEntryId = -1;
    selector->playerCountBpObject = 0;
    selector->detailRequested = 0;
    selector->detailActive = 0;
    selector->detailOpenStarted = 0;
    selector->detailCloseStarted = 0;
    selector->detailCloseRequested = 0;
    selector->detailBpObject = 0;
    selector->detailBpEntryId = -1;
    selector->detailLayout = 0;
    selector->detailItemIndex = -1;
    selector->detailItemDisplay = 0;
    selector->textRenderer = 0;
    void* selectionLayout = __nw__FUl(kLayoutObjectBytes);
    if (selectionLayout)
        Layout_Init(selectionLayout, kLayoutInitMode);
    selector->selectionLayout = selectionLayout;
    Layout_InitStandard(selectionLayout, 0, s_partyItemSelectionLayoutElements, kSelectionElementCount,
                        partyItemSelectStrings);
    Layout_SetActive((void*)selector->selectionLayout, 1);

    // Build the shared render context on first use. Record destruction is shared
    // with enemy rendering: both layouts have string words +54/+58/+5C and an
    // owned renderObject at +A0 in a 0xA4-byte record. Other fields need not agree.
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            char* base = (char*)s_renderContext;
            char* playerCursor = base;
            char* playerEnd = base + kRenderVectorsOffset;
            while (playerCursor < playerEnd) {
                SaveData_InitPlayerRecord(playerCursor);
                playerCursor += sizeof(SavePlayerRecord);
            }
            ((IntVectorArray*)(playerEnd))->__ct();
            playerCursor = base + kRenderSelectionsOffset;
            __construct_array(playerCursor, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor,
                              sizeof(PartyItemRecord), kMaxPlayers);
            ((PartyItemSelection*)playerCursor)->count = 0;
            try {
                RenderContext_ResetPlayerRecords(base);
            } catch (...) {
                ((void (*)(void*, int))CharacterPreviewTable_dtor)(base + kRenderSelectionsOffset, -1);
                ((IntVectorArray*)(base + kRenderVectorsOffset))->~IntVectorArray();
                throw;
            }
            __register_global_object(base, (void*)__dt__13RenderContextFv, s_partyItemSelectRenderContextRegistration);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
    int* renderContextWords = (int*)((int&)Sing_805DC648());
    selector->selectionBpEntryId = 0;
    selector->playerSelections = &((RenderContext*)renderContextWords)->players;

    // Register selection and player-count scripts, then cache their borrowed objects.
    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry((const char*)partyItemSelectStrings + kSelectionScriptOffset, &selector->selectionBpEntryId, 0);

    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry((const char*)partyItemSelectStrings + kPlayerCountScriptOffset, &selector->playerCountBpEntryId,
                      0);

    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    selector->selectionBpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), selector->selectionBpEntryId);

    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    selector->playerCountBpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), selector->playerCountBpEntryId);

    // Allocate one inactive item/activation pair per participating player.
    {
        int playerCount = ((PartyItemSelection*)selector->playerSelections)->count;
        void* activationStorage = __nwa__FUl(playerCount * (int)sizeof(PartyItemActivation));
        selector->playerActivations = activationStorage;
        int activationPlayer = 0;
        int activationByteOffset = 0;
        while (activationPlayer < ((PartyItemSelection*)selector->playerSelections)->count) {
            // First word is the UI element ID (-1); +4 is feedback frames remaining (0).
            // Word indexing preserves the signed shift used by the original code.
            ((int*)activationStorage)[activationByteOffset >> 2] = -1;
            PartyItemActivation* activation = (PartyItemActivation*)((char*)activationStorage + activationByteOffset);
            activationPlayer = activationPlayer + 1;
            activationByteOffset = activationByteOffset + (int)sizeof(PartyItemActivation);
            activation->feedbackFramesRemaining = 0;
        }
    }

    // Allocate the per-player item-display pointer slots.
    {
        int playerCount = ((PartyItemSelection*)selector->playerSelections)->count;
        void* displayStorage = __nwa__FUl(playerCount * (int)sizeof(void*));
        selector->selectedItemDisplays = displayStorage;
        int displayPlayer = 0;
        int displayByteOffset = 0;
        int emptyDisplay = 0;
        while (displayPlayer < ((PartyItemSelection*)selector->playerSelections)->count) {
            // These are four-byte pointer slots on Wii; integer zero is null.
            ((int*)displayStorage)[displayByteOffset >> 2] = emptyDisplay;
            displayPlayer = displayPlayer + 1;
            displayByteOffset = displayByteOffset + (int)sizeof(void*);
        }
    }

    // Reuse the shared chat font for item name and description text.
    {
        void* chatMessage = (void*)g_chatMsg;
        ChatFontSlot* chat = (ChatFontSlot*)chatMessage;
        ((SelectorFontSlot*)this)->font = chat->font;
        WFixedFont_CreateAndRegisterMsgDraw(((SelectorFontSlot*)this)->font, &selector->textRenderer, 1);
        WFixedFont_CreateMsgEcbResource(((SelectorFontSlot*)this)->font, &selector->textTemplates, 0, 0);
    }

    // Build the common item-details layout.
    void* detailLayout = __nw__FUl(kLayoutObjectBytes);
    if (detailLayout)
        Layout_Init(detailLayout, kLayoutInitMode);
    selector->detailLayout = detailLayout;
    Layout_InitStandard(detailLayout, 0, s_partyItemDetailLayoutElements, kDetailElementCount,
                        (const char*)partyItemSelectStrings + kDetailLayoutOffset);

    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry((const char*)partyItemSelectStrings + kDetailScriptOffset, &selector->detailBpEntryId, 0);

    if (((int&)Sing_805DC5F0()) == 0) {
        PartySelector_RegisterBpController();
    }
    selector->detailBpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), selector->detailBpEntryId);
}

} // extern "C"
namespace std {
// Construct an empty per-player integer vector.
vector<int, app::app_allocator<int> >::vector() : items(0), size(0), capacity(0) {}
// Release this vector through the game allocator after clearing its count.
vector<int, app::app_allocator<int> >::~vector() {
    // Logical operation: clear size and free items. The original inlined member
    // destructors each test this; their repeated checks and mixed-type subtraction
    // preserve generated code. Words are {items, size, capacity}; subtraction is 0.
    if (this) {
        if (this) {
            if (this) {
                if (this) {
                    int* fields = (int*)this;
                    if (fields[0]) {
                        fields[1] = *(unsigned int*)&fields[1] - *(int*)&fields[1];
                        appFreeUni__FPv((void*)fields[0]);
                    }
                }
            }
        }
    }
}
}
