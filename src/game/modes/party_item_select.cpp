extern "C" char s_bpCtrlRegistration_804EC728[];
extern "C" char s_partyItemSelectBpHandleRegistration[];
extern "C" char s_partyItemSelectRenderContextRegistration[];
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "classes/PartyItemActivation.h"
#include "classes/CMsgDraw.h"
#include "classes/PartyItemSelection.h"
#include "classes/PartyItemRecord.h"
#include "classes/SavePlayerRecord.h"
#include "game/vector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);
// Translation unit 0x800893A0-0x8008B524 (12 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#define __register_global_object __sdk_register_global_object
#include "sdk_protos.h"
#include "classes/CPartyItemSelect.h"

#define AsCPartyItemSelect(p) ((CPartyItemSelect*)(p))
#undef __register_global_object
struct Layout; // Layout operations use opaque handles; CPartyItemSelect owns both instances.

extern "C" {
void Layout_Update(Layout* layout);

void CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);
void __destroy_arr(void*, void (*)(void*), int, int);
void __dl__FPv(void*);
void CCharacterPreviewRecord_CopyPreservingRenderer(void*, void*);
unsigned Layout_PollLoadComplete(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*, int);
void* __register_global_object(void*, void (*)(void*, int), void*);
unsigned CBpCtrl_PollAllEntriesLoaded(void*);

extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_renderContextInitialized;
extern signed char s_pointerPairListInitialized;
extern char s_bpCtrl[];
extern char s_partyItemSelectSingletonRegistrations[];
extern char s_renderContext[];
extern char s_messageDrawPairList[];
extern char __vt__16CPartyItemSelect[];
extern char s_bpCtrlRegistration_804EC728[];
// Named entries retain their original storage and relocation addends.
struct PartyItemSelectStringPool {
    char selectionScripts[0x16];
    char selectionLayout[0x16];
    char noEntryLayout[0x17];
    char detailScripts[0x10];
    char detailLayout[0x10];
    char openSound[0xf];
    char openDetailScript[0xf];
    char closeSound[0x10];
    char closeDetailScript[0x10];
    char thirdPlayerGroup[0x8];
    char fourthPlayerGroup[0x8];
    char twoPlayerScript[0xb];
    char threePlayerScript[0xb];
    char fourPlayerScript[0xb];
    char selectionScript[0x12];
    char itemNameTextStyle[0x12];
    char itemExplanationTextStyle[0x10];
};
extern PartyItemSelectStringPool partyItemSelectStrings;
typedef char PartyItemStrings_name[((int)&((PartyItemSelectStringPool*)0)->itemNameTextStyle == 0xE4) ? 1 : -1];
// Selection layout map: item n in [0,12) uses button 8+n, image 20+n,
// highlight 32+n and highlight image 44+n; continue is element 56.
// Detail layout: 1 explanation placement, 2 name placement; element 0 artwork is unresolved.
// Detail state progression: requested -> active -> openStarted -> closeRequested
// -> closeStarted -> all flags cleared when the closing script finishes.
// PartyItemActivation::feedbackFramesRemaining stores remaining feedback frames.
enum {
    kPartyPlayerCount = 4, kPartyItemCount = 12,
    kPartyItemButtonBase = 8, kPartyItemImageBase = 20,
    kPartyItemHighlightBase = 32, kPartyItemHighlightImageBase = 44,
    kPartyContinueElement = 56, kPartySelectScriptIndex = 11,
    kActionCancel = 6, kActionItemDetails = 20,
    kNoItemSelection = -1, kSelectionFeedbackFrames = 4,
    kPartyActivationStride = (int)sizeof(PartyItemActivation),
    kFadeInputBlockedState1 = 1, kFadeInputBlockedState3 = 3,
    kPartyRecordRetainedFlags = 0x0FFF,
    // Preserve pointer-addition grouping in the constructor\'s exception cleanup.
    kRenderContextVectorsOffset = (int)&((RenderContext*)0)->vectors,
    kRenderRecordRendererOffset = (int)&((RenderContext*)0)->players + (int)&((PartyItemRecord*)0)->renderObject
};
extern char s_partyItemDetailTextSeed[];
extern void* s_partyItemNameTemplates[];
extern void* s_partyItemExplanationTemplates[];

void* fn_8008AFF0_layout_placeholder;
void String_ReserveCapacity(void*, unsigned int);
void CMsgDraw_ResetTextState(void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
unsigned UIGroup_GetTintColor(void*);
int UIGroup_GetCombinedOpacity(void*);
void Layout_GetElementPosition(void*, int, void*);
void Text_SetPosition(void*, void*);
void Layout_GetElementSize(void*, int, void*);
int CMsgDraw_SetLayoutSize(void*, void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void WString_ReplaceRange(void*, int, int, void*, void*, unsigned char*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Text_SetContent(void*, void*);
void appFreeUni__FPv(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
int BpObject_IsCurrentScriptComplete(void*);
void Layout_DrawWithOpacity(void*, float);
void* RenderContext_GetOrCreate(void);
void CCharacterPreviewTable_DrawAvatars(void*);
void CMsgDraw_UpdateReveal(void*);
void Text_DrawGlyphs(void*);
int GX_GetPrimitiveCount(int, int);
void Layout_ReadElementRect(void*, int, void*);
void DrawPrimitiveUP(int, int, void*, int);
void CBpCtrlHandle_Dtor(void*);
void CBpCtrlHandle_ReleaseEntry(void*);
void SaveData_InitPlayerRecord(void*);
void* PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void CCharacterPreviewTable_SetScriptEnabled(void*, int);
void __dla__FPv(void*);
void Layout_Destroy(void*, int);
void CBpCtrl_DeleteEntryById(void*, int);
void PairVector_InitReserved(void*);
void PointerPairList_RemoveByKey(void*, void*);
void* __dt__13RenderContextFv(void*, int);
void* MessageDrawPairList_Dtor(void*, int);
void* CBpCtrlHandle_SetScriptIndex(void*, int);
void CCharacterPreviewTable_SetupRecordAvatar(void*, int, void*, int, float);
void CAsyncExhibition_SetLoadingIndicatorPosition(void*, void*);
void Layout_WriteElementRect(void*, int, void*);
void Layout_SetElementEnabled(void*, int, void*);
void Layout_SetElementScale(void*, int, void*);
void Layout_SetElementPosition(void*, int, void*);
void UIGroup_SetEnabled(void*, int);
void BpObject_StartScriptByName(void*, const char*, int);
void IFadeMenuBase_StartMenuFadeOut(void*);
extern int s_partyItemSelectionHighlightQuad[24];
extern int s_partyItemDetailHighlightQuad[24];
extern char s_partyItemSelectRenderContextRegistration[];
extern char s_partyItemSelectBpHandleRegistration[];
extern char s_partyItemAvatarSetupParameters[];
extern char s_partyItemAvatarLoadingPositions[];
extern int kPartyItemHighlightScaleX;
extern int kPartyItemHighlightScaleY;
extern float kPartyItemAvatarSetupScale;
extern float kPartyItemHighlightPositionInset;
extern float kPartyItemPlayerIndex1OffsetX;
extern float kPartyItemPlayerIndex2OffsetX;
extern float kPartyItemPlayerIndex3OffsetX;

struct ReleasedVectorStorage { void* data; int count; int capacity; };

struct Fn800893A0Virtual {
    virtual void destroy(int);
};

#pragma push
#pragma use_lmw_stmw on
// Ensure the borrowed BP controller exists; menu entries remain owned by their IDs.
static inline void EnsurePartyItemScriptController(char* names) {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor,
                (names + 0x18));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Release layouts, script entries, player feedback buffers and text renderers owned by the menu.
void* CPartyItemSelect_dtor(CPartyItemSelect* self, int flag) {
    // Ownership/order: release caption handle; disable borrowed render-context scripts;
    // free selected-item displays and selection layout; delete BP entries when loaded;
    // free player activation storage; destroy detail layout; unregister text-renderer pair;
    // destroy text renderer/templates; delete detail BP entry; finally delete self if flag>0.
    // Singleton initialization blocks obtain borrowed services, never transfer them to self.
    // Reading guide: each `if (Sing_XXXX() == 0) { construct; __register_global_object; flag = 1;
    // Sing_XXXX() = obj; }` block is one inlined lazy getter for a global singleton (BP controller
    // Sing_805DC5F0, render context Sing_805DC648, pair list Sing_805DC650). It only guarantees
    // the object exists before the next call uses it; the release order is the list above.
    char* names = s_partyItemSelectSingletonRegistrations;
    if (self != 0) {
        void* cached = Sing_805DC5F8();
        self->vtbl = __vt__16CPartyItemSelect;

        if (cached == 0) {
            if (s_bpCtrlHandleInitialized == 0) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle,
                    (void (*)(void*, int))CBpCtrlHandle_Dtor,
                    (names + 0x30));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());

        if (Sing_805DC648() == 0) {
            if (s_renderContextInitialized == 0) {
                char* end = (char*)&((RenderContext*)s_renderContext)->vectors;
                char* p = s_renderContext;
                char* base = s_renderContext;
                do {
                    SaveData_InitPlayerRecord(p);
                    p += sizeof(SavePlayerRecord);
                } while (p < end);
                ((IntVectorArray*)(end))->__ct();
                char* member = ((RenderContext*)base)->players;
                __construct_array(member, (void*)PartyItemRecord_InitDefaults,
                    (void*)PlayerRenderRecord_dtor, sizeof(PartyItemRecord), kPartyPlayerCount);
                ((PartyItemSelection*)member)->count = 0;
                try {
                    RenderContext_ResetPlayerRecords(base);
                } catch (...) {
                    ((void (*)(void*, int))CharacterPreviewTable_dtor)(((RenderContext*)base)->players, -1);
                    ((IntVectorArray*)((char*)&((RenderContext*)base)->vectors))->~IntVectorArray();
                    throw;
                }
                __register_global_object(s_renderContext,
                    (void (*)(void*, int))__dt__13RenderContextFv, (names + 0x24));
                s_renderContextInitialized = 1;
            }
            Sing_805DC648() = s_renderContext;
        }
        CCharacterPreviewTable_SetScriptEnabled(((RenderContext*)Sing_805DC648())->players, 0);

        // Owned selection display storage and layout can now be released.
        if (self->selectedItemDisplays != 0) {
            __dla__FPv(self->selectedItemDisplays);
            self->selectedItemDisplays = 0;
        }
        if (self->selectionLayout != 0) {
            Layout_Destroy(self->selectionLayout, 1);
            self->selectionLayout = 0;
        }

        EnsurePartyItemScriptController(names);
        if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) != 0) {
            if (self->selectionBpEntryId != -1) {
                EnsurePartyItemScriptController(names);
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->selectionBpEntryId);
                self->selectionBpEntryId = -1;
            }
            if (self->playerCountBpEntryId != -1) {
                EnsurePartyItemScriptController(names);
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->playerCountBpEntryId);
                self->playerCountBpEntryId = -1;
            }
        }

        if (self->playerActivations != 0) {
            __dla__FPv(self->playerActivations);
            self->playerActivations = 0;
        }

        if (Sing_805DC650() == 0) {
            if (s_pointerPairListInitialized == 0) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList,
                    (void (*)(void*, int))MessageDrawPairList_Dtor,
                    (names + 0x3C));
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        void* singleton = Sing_805DC650();
        if (self->detailLayout != 0) {
            Layout_Destroy(self->detailLayout, 1);
            self->detailLayout = 0;
        }
        // Remove the borrowed registry reference BEFORE destroying its owned renderer.
        PointerPairList_RemoveByKey(singleton, self->textRenderer);

        if (self->textRenderer != 0) {
            if (self->textRenderer != 0) {
                ((Fn800893A0Virtual*)self->textRenderer)->destroy(1);
            }
            self->textRenderer = 0;
        }
        if (self->textTemplates != 0) {
            if (self->textTemplates != 0) {
                ((Fn800893A0Virtual*)self->textTemplates)->destroy(1);
            }
            self->textTemplates = 0;
        }

        if (self->detailBpEntryId != -1) {
            EnsurePartyItemScriptController(names);
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->detailBpEntryId);
            self->detailBpEntryId = -1;
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Update the selection and item-detail layouts.
void CPartyItemSelect_UpdateLayouts(CPartyItemSelect* self) {
    Layout_Update((Layout*)self->selectionLayout);
    Layout_Update((Layout*)self->detailLayout);
}

} // extern "C"
// Destroy the embedded player table; implicit C++ teardown then destroys the four vectors.
RenderContext::~RenderContext() {
    CharacterPreviewTable_dtor(players, -1);
}
extern "C" {

// Release vector storage and optionally delete the vector object; retain the original cleanup shape.
void* dtor_8008A5B4(MSLPointerVector* self, int flag) {
    return ::std::__vector_deleting_dtor(self, flag);
}

// Release the message-renderer pointer-pair list storage and optionally delete its object.
void* MessageDrawPairList_Dtor(void* p, int flag) {
    return ::std::__vector_deleting_dtor((MSLPointerVector*)p, flag);
}

// Copy all four player records and their count only after the menu has completed selection.
int CPartyItemSelect_CopyCompletedSelection(void* self, void* dst) {
    if (((CPartyItemSelect*)self)->selectionComplete != 0) {
        void* src = ((CPartyItemSelect*)self)->playerSelections;
        CCharacterPreviewRecord_CopyPreservingRenderer(dst, src);
        CCharacterPreviewRecord_CopyPreservingRenderer(&((PartyItemSelection*)dst)->records[1], &((PartyItemSelection*)src)->records[1]);
        CCharacterPreviewRecord_CopyPreservingRenderer(&((PartyItemSelection*)dst)->records[2], &((PartyItemSelection*)src)->records[2]);
        CCharacterPreviewRecord_CopyPreservingRenderer(&((PartyItemSelection*)dst)->records[3], &((PartyItemSelection*)src)->records[3]);
        ((PartyItemSelection*)dst)->count = ((PartyItemSelection*)src)->count;
        return 1;
    }
    return 0;
}

// Poll both layouts and all script entries; return true when every resource is loaded.
unsigned CPartyItemSelect_PollLoadComplete(void* self) {
    unsigned result = Layout_PollLoadComplete(AsCPartyItemSelect(self)->selectionLayout) & 1;
    result &= Layout_PollLoadComplete(AsCPartyItemSelect(self)->detailLayout);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlRegistration_804EC728);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    result &= CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0());
    return result;
}

// Initialize inventory sentinels, equipment/name/render state, and clear the high four record flags.
void* PartyItemRecord_InitDefaults(void* self) {
    unsigned short retainedFlags = ((PartyItemRecord*)self)->flags;
    int zero = 0;
    unsigned int invalidInventoryIndex = -1;

    ((PartyItemRecord*)self)->hairStyleInventoryIndex = zero;
     retainedFlags = (unsigned short)(retainedFlags & kPartyRecordRetainedFlags);
    ((PartyItemRecord*)self)->unk_8 = zero;
      ((PartyItemRecord*)self)->characterInventoryIndex = invalidInventoryIndex;
    ((PartyItemRecord*)self)->characterIndex = zero;
    // The following two shared fields hold club-set and ball inventory indices.
    ((PartyItemRecord*)self)->clubSetInventoryIndex = zero;
    ((PartyItemRecord*)self)->ballInventoryIndex = zero;
    ((PartyItemRecord*)self)->playerIndex = zero;
    ((PartyItemRecord*)self)->unk_1D = -1;
    ((PartyItemRecord*)self)->levelIndex = zero;
    ((PartyItemRecord*)self)->flags = retainedFlags;
    ((PartyItemRecord*)self)->nameFlags = zero;
    ((PartyItemRecord*)self)->nameLength = zero;
    ((PartyItemRecord*)self)->nameData = zero;
    ((PartyItemRecord*)self)->unk_88 = zero;
    ((PartyItemRecord*)self)->unk_8C = zero;
    ((PartyItemRecord*)self)->renderObject = zero;
    ((PartyItemRecord*)self)->partInventoryIndex0 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex1 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex2 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex3 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex4 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex5 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex6 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex7 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex8 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex9 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex10 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex11 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex12 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex13 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex14 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex15 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex16 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex17 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex18 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex19 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex20 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex21 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex22 = -1;
    ((PartyItemRecord*)self)->partInventoryIndex23 = -1;
    ((PartyItemRecord*)self)->unk_8D = 0;
    ((PartyItemRecord*)self)->unk_8E = 0;
    ((PartyItemRecord*)self)->unk_8F = 0;
    ((PartyItemRecord*)self)->unk_90 = 0;
    ((PartyItemRecord*)self)->unk_91 = 0;
    ((PartyItemRecord*)self)->unk_92 = 0;
    ((PartyItemRecord*)self)->unk_93 = 0;
    ((PartyItemRecord*)self)->unk_94 = 0;
    ((PartyItemRecord*)self)->unk_95 = 0;
    ((PartyItemRecord*)self)->unk_96 = 0;
    ((PartyItemRecord*)self)->unk_97 = 0;
    ((PartyItemRecord*)self)->unk_98 = 0;
    ((PartyItemRecord*)self)->unk_99 = 0;
    ((PartyItemRecord*)self)->unk_9A = 0;
    ((PartyItemRecord*)self)->unk_68 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_6C = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_70 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_74 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_78 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_7C = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_80 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->unk_84 = invalidInventoryIndex;
    ((PartyItemRecord*)self)->fallbackPower = (unsigned char)-1;
    ((PartyItemRecord*)self)->fallbackControl = (unsigned char)-1;
    ((PartyItemRecord*)self)->fallbackAccuracy = (unsigned char)-1;
    ((PartyItemRecord*)self)->fallbackSpin = (unsigned char)-1;
    ((PartyItemRecord*)self)->fallbackCurve = (unsigned char)-1;
    ((PartyItemRecord*)self)->equipmentPowerBonus = (unsigned char)-1;
    ((PartyItemRecord*)self)->equipmentControlBonus = (unsigned char)-1;
    ((PartyItemRecord*)self)->equipmentAccuracyBonus = (unsigned char)-1;
    ((PartyItemRecord*)self)->equipmentSpinBonus = (unsigned char)-1;
    ((PartyItemRecord*)self)->equipmentCurveBonus = (unsigned char)-1;
    String_ReserveCapacity(&((PartyItemRecord*)self)->nameFlags, 0x20);
    return self;
}

struct WStrTmp {
    unsigned int flags;
    unsigned int size;
    void* data;
    // Release this temporary wide string only when its tag selects heap ownership.
    ~WStrTmp() { if (flags >> 31) appFreeUni__FPv(data); }
};

struct WStrAllocator {};

// Render the selected item\'s name and explanation using each layout text style and position.
void CPartyItemSelect_RefreshDetailText(void* self) {
    CMsgDraw_ResetTextState(((CPartyItemSelect*)self)->textRenderer);
    if (((CPartyItemSelect*)self)->detailCloseRequested != 0) return;

    float textPosition[2];
    float textSize[2];
    WStrAllocator u18;
    WStrAllocator b18;
    unsigned char byte14[4];
    volatile int u10;
    unsigned char byte0c[4];
    volatile int u08;
    void* style = BpObject_GetOrCreateNamedGroup(((CPartyItemSelect*)self)->detailBpObject, partyItemSelectStrings.itemNameTextStyle);
    unsigned color = UIGroup_GetTintColor(style);
    unsigned int flag = UIGroup_GetCombinedOpacity(style);
    // UI tint low bytes are B,G,R; shifts repack them as renderer R,G,B,A.
    // Effective opacity maps 0->0 and n>0 -> 2*n-1 in the low alpha byte.
    // Preserve this byte permutation; passing the packed UI color directly changes channels.
    unsigned int alpha = flag ? 2 * flag - 1 : 0;
    ((CMsgDraw*)((CPartyItemSelect*)self)->textRenderer)->colorMultiplier = alpha | ((color << 24) | (color << 8 & 0xff0000) | (color >> 8 & 0xff00)); // 0x8008B028: renderer +0x68

    // Phase 1: build the item name in an owned temporary wide string. Templates, seed
    // and selected inline/heap character pointers are borrowed only during these calls.
    WStrTmp detailText;
    detailText.flags = 0;
    detailText.size = 0;
    detailText.data = 0;
    Layout_GetElementPosition(((CPartyItemSelect*)self)->detailLayout, 2, textPosition);
    Text_SetPosition(((CPartyItemSelect*)self)->textRenderer, textPosition);
    Layout_GetElementSize(((CPartyItemSelect*)self)->detailLayout, 2, textSize);
    CMsgDraw_SetLayoutSize(((CPartyItemSelect*)self)->textRenderer, textSize);
    CMsgDraw_ClearFormattingAndRevealAll(((CPartyItemSelect*)self)->textRenderer);

    int currentLength;
    void* s = s_partyItemDetailTextSeed;
    if ((detailText.flags >> 31) == 0) {
        currentLength = (*(unsigned char*)&detailText) & 0x7F;
    } else {
        currentLength = detailText.size;
    }
    b18 = u18;
    int t = wcslen((const wchar_t*)s);
    WString_ReplaceRange(&detailText, 0, currentLength, s, (char*)s + t * 2, (unsigned char*)&b18);

    int idx = ((CPartyItemSelect*)self)->detailItemIndex;
    void** arr = s_partyItemNameTemplates;
    Text_ExpandTaggedText(((CPartyItemSelect*)self)->textTemplates, &detailText, arr[idx], 0);

    void* itemName;
    if ((detailText.flags >> 31) == 0) {
        itemName = (char*)&detailText + 2;
    } else {
        itemName = detailText.data;
    }
    Text_SetContent(((CPartyItemSelect*)self)->textRenderer, itemName);
    byte14[0] = *(volatile unsigned char*)&u10;
    WString_ReplaceRange(&detailText, 0, -1, 0, 0, byte14);

    // Phase 2: clear/reuse that temporary, apply explanation style/position and expand text.
    // Text_SetContent must consume/copy characters before the temporary is cleared below.
    void* style2 = BpObject_GetOrCreateNamedGroup(((CPartyItemSelect*)self)->detailBpObject, partyItemSelectStrings.itemExplanationTextStyle);
    unsigned color2 = UIGroup_GetTintColor(style2);
    unsigned int flag2 = UIGroup_GetCombinedOpacity(style2);
    unsigned int alpha2 = flag2 ? 2 * flag2 - 1 : 0;
    ((CMsgDraw*)((CPartyItemSelect*)self)->textRenderer)->colorMultiplier = alpha2 | ((color2 << 24) | (color2 << 8 & 0xff0000) | (color2 >> 8 & 0xff00)); // 0x8008B028: renderer +0x68

    Layout_GetElementPosition(((CPartyItemSelect*)self)->detailLayout, 1, textPosition);
    Text_SetPosition(((CPartyItemSelect*)self)->textRenderer, textPosition);
    Text_ExpandTaggedText(((CPartyItemSelect*)self)->textTemplates, &detailText, ((void**)s_partyItemExplanationTemplates)[((CPartyItemSelect*)self)->detailItemIndex], 0);

    void* explanation;
    if ((detailText.flags >> 31) == 0) {
        explanation = (char*)&detailText + 2;
    } else {
        explanation = detailText.data;
    }
    Text_SetContent(((CPartyItemSelect*)self)->textRenderer, explanation);
    byte0c[0] = *(volatile unsigned char*)&u08;
    WString_ReplaceRange(&detailText, 0, -1, 0, 0, byte0c);
}

struct Tmp96 { int data[24]; };
struct Draw8008A6B8 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void draw();
    virtual int ready();
};
// The UI quad uses the same 24-byte position/color/UV vertex layout as the title menus.
struct PartyItemVertex {
    float x, y, z;
    unsigned char red, green, blue, alpha;
    float u, v;
};
struct Quad8008A6B8 { PartyItemVertex vertices[4]; };
typedef char PartyItemVertex_size[(sizeof(PartyItemVertex) == 0x18) ? 1 : -1];
struct Self8008A6B8 { void render(); };
extern float kFloat_1_PartyItemSelect;

// Draw player scripts, ready item previews, highlight quads, avatars and the active detail text.
inline void Self8008A6B8::render() {
    char* self = (char*)this;
    int playerSlot;
    int detailRect[4];
    int selectionRect[4];
    Quad8008A6B8 detailQuad;
    Quad8008A6B8 selectionQuad;
    BpObject_TryInvokeCurrentCallback(((CPartyItemSelect*)self)->selectionBpObject, 0);
    BpObject_Draw(((CPartyItemSelect*)self)->selectionBpObject);
    BpObject_TryInvokeCurrentCallback(((CPartyItemSelect*)self)->playerCountBpObject, 0);
    BpObject_Draw(((CPartyItemSelect*)self)->playerCountBpObject);
    if (BpObject_IsCurrentScriptComplete(((CPartyItemSelect*)self)->selectionBpObject) == 1) {
        Layout_DrawWithOpacity(((CPartyItemSelect*)self)->selectionLayout, kFloat_1_PartyItemSelect);
        for (playerSlot = 0; playerSlot < ((PartyItemSelection*)((CPartyItemSelect*)self)->playerSelections)->count; playerSlot++) {
            if ((*(Draw8008A6B8***)&((CPartyItemSelect*)self)->selectedItemDisplays)[playerSlot] != 0) {
                if ((*(Draw8008A6B8***)&((CPartyItemSelect*)self)->selectedItemDisplays)[playerSlot]->ready() != 0) {
                    (*(Draw8008A6B8***)&((CPartyItemSelect*)self)->selectedItemDisplays)[playerSlot]->draw();
                    Layout_ReadElementRect(((CPartyItemSelect*)self)->selectionLayout, playerSlot, selectionRect);
                    selectionQuad = *(Quad8008A6B8*)s_partyItemSelectionHighlightQuad;
                    selectionQuad.vertices[0].x = selectionRect[0];
                    selectionQuad.vertices[0].y = selectionRect[1];
                    selectionQuad.vertices[1].x = selectionRect[2];
                    selectionQuad.vertices[1].y = selectionRect[1];
                    selectionQuad.vertices[2].x = selectionRect[0];
                    selectionQuad.vertices[2].y = selectionRect[3];
                    selectionQuad.vertices[3].x = selectionRect[2];
                    selectionQuad.vertices[3].y = selectionRect[3];
                    int cnt = GX_GetPrimitiveCount(4, 0x98);
                    DrawPrimitiveUP(0x98, cnt, &selectionQuad, 0x18);
                }
            }
        }
    }
    void* p2 = RenderContext_GetOrCreate();
    CCharacterPreviewTable_DrawAvatars(((RenderContext*)p2)->players);
    if (((CPartyItemSelect*)self)->detailActive != 0) {
        if (BpObject_IsCurrentScriptComplete(((CPartyItemSelect*)self)->detailBpObject) != 0) {
            BpObject_TryInvokeCurrentCallback(((CPartyItemSelect*)self)->detailBpObject, 0);
            BpObject_Draw(((CPartyItemSelect*)self)->detailBpObject);
            Layout_DrawWithOpacity(((CPartyItemSelect*)self)->detailLayout, kFloat_1_PartyItemSelect);
            if (((CPartyItemSelect*)self)->detailItemDisplay != 0) {
                if ((*(Draw8008A6B8**)&((CPartyItemSelect*)self)->detailItemDisplay)->ready() != 0) {
                    (*(Draw8008A6B8**)&((CPartyItemSelect*)self)->detailItemDisplay)->draw();
                    Layout_ReadElementRect(((CPartyItemSelect*)self)->detailLayout, 0, detailRect);
                    detailQuad = *(Quad8008A6B8*)s_partyItemDetailHighlightQuad;
                    detailQuad.vertices[0].x = detailRect[0];
                    detailQuad.vertices[0].y = detailRect[1];
                    detailQuad.vertices[1].x = detailRect[2];
                    detailQuad.vertices[1].y = detailRect[1];
                    detailQuad.vertices[2].x = detailRect[0];
                    detailQuad.vertices[2].y = detailRect[3];
                    detailQuad.vertices[3].x = detailRect[2];
                    detailQuad.vertices[3].y = detailRect[3];
                    int cnt2 = GX_GetPrimitiveCount(4, 0x98);
                    DrawPrimitiveUP(0x98, cnt2, &detailQuad, 0x18);
                }
            }
        } else {
            BpObject_TryInvokeCurrentCallback(((CPartyItemSelect*)self)->detailBpObject, 0);
            BpObject_Draw(((CPartyItemSelect*)self)->detailBpObject);
        }
        CMsgDraw_UpdateReveal(((CPartyItemSelect*)self)->textRenderer);
        Text_DrawGlyphs(((CPartyItemSelect*)self)->textRenderer);
    }
}

// Draw the selection menu and its optional item-detail overlay.
void CPartyItemSelect_Draw(Self8008A6B8* self) { self->render(); }

// Initialize player avatars, duplicated item hit rectangles and the player-count script.
void CPartyItemSelect_InitializeSelectionLayout(CPartyItemSelect* self) {
    if (Sing_805DC5F8() == 0) {
        if (s_bpCtrlHandleInitialized == 0) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle,
                                     (void (*)(void*, int))CBpCtrlHandle_Dtor,
                                     s_partyItemSelectBpHandleRegistration);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kPartySelectScriptIndex);

    float add33;
    float add22;
    float add11;
    char* base;
    char* p;
    int offset;
    char* config;
    char* name;
    int index;
    config = s_partyItemAvatarSetupParameters;
    name = s_partyItemAvatarLoadingPositions;
    float adjustment = kPartyItemAvatarSetupScale;
    index = 0;
    offset = 0;
    while (index < ((PartyItemSelection*)self->playerSelections)->count) {
        ((PartyItemRecord*)((char*)self->playerSelections + offset))->selection = kNoItemSelection;
        CCharacterPreviewTable_SetupRecordAvatar(((RenderContext*)RenderContext_GetOrCreate())->players, index,
                    config, 0, adjustment);
        if (*(void**)((char*)RenderContext_GetOrCreate() + offset + kRenderRecordRendererOffset) != 0) {
            if (Sing_805DC648() == 0) {
                if (s_renderContextInitialized == 0) {
                    base = s_renderContext;
                    p = base;
                    do {
                        SaveData_InitPlayerRecord(p);
                        p += sizeof(SavePlayerRecord);
                    } while (p < base + kRenderContextVectorsOffset);
                    ((IntVectorArray*)(base + kRenderContextVectorsOffset))->__ct();
                    p = ((RenderContext*)base)->players;
                    __construct_array(p, (void (*)(void*))PartyItemRecord_InitDefaults,
                                PlayerRenderRecord_dtor, sizeof(PartyItemRecord), kPartyPlayerCount);
                    ((PartyItemSelection*)p)->count = 0;
                    try {
                        RenderContext_ResetPlayerRecords(base);
                    } catch (...) {
                        ((void (*)(void*, int))CharacterPreviewTable_dtor)(((RenderContext*)base)->players, -1);
                        ((IntVectorArray*)(base + kRenderContextVectorsOffset))->~IntVectorArray();
                        throw;
                    }
                    __register_global_object(s_renderContext,
                                             (void (*)(void*, int))__dt__13RenderContextFv,
                                             s_partyItemSelectRenderContextRegistration);
                    s_renderContextInitialized = 1;
                }
                Sing_805DC648() = s_renderContext;
            }
            CAsyncExhibition_SetLoadingIndicatorPosition(*(void**)((char*)Sing_805DC648() + offset + kRenderRecordRendererOffset),
                        name);
        }
        offset += sizeof(PartyItemRecord);
        config += 0x10;
        name += 8;
        ++index;
    }

    // Selection layout element map (12 items per group, indices from the enum above):
    //   0..3 per-player elements (disabled here)   4..7 per-player markers (moved/disabled below)
    //   8..19 item buttons (kPartyItemButtonBase)  20..31 item images (kPartyItemImageBase)
    //   32..43 item highlights (kPartyItemHighlightBase)  44..55 highlight images
    //   (kPartyItemHighlightImageBase). Hit rectangles of the buttons are duplicated onto the
    //   image (+12) and highlight-image (+36) elements so all three respond to the same area.
    int value[4]; // Layout_ReadElementRect/WriteElementRect move 4 ints
    for (int i = kPartyItemButtonBase; i <= kPartyItemButtonBase + kPartyItemCount - 1; ++i) {
        Layout_ReadElementRect(self->selectionLayout, i, value);
        Layout_WriteElementRect(self->selectionLayout, i + (kPartyItemImageBase - kPartyItemButtonBase), value);
        Layout_WriteElementRect(self->selectionLayout, i + (kPartyItemHighlightImageBase - kPartyItemButtonBase), value);
    }
    for (int i = 0; i <= 3; ++i)
        Layout_SetElementEnabled(self->selectionLayout, i, 0);
    for (int i = kPartyItemHighlightBase; i <= kPartyItemHighlightBase + kPartyItemCount - 1; ++i)
        Layout_SetElementEnabled(self->selectionLayout, i, 0);

    int initial[2];
    initial[0] = kPartyItemHighlightScaleX;
    initial[1] = kPartyItemHighlightScaleY;
    float subtract = kPartyItemHighlightPositionInset;
    float vec[2];
    for (int i = kPartyItemHighlightImageBase; i <= kPartyItemHighlightImageBase + kPartyItemCount - 1; ++i) {
        Layout_SetElementEnabled(self->selectionLayout, i, 0);
        Layout_SetElementScale(self->selectionLayout, i, initial);
        Layout_GetElementPosition(self->selectionLayout, i, vec);
        vec[0] -= subtract;
        vec[1] -= subtract;
        Layout_SetElementPosition(self->selectionLayout, i, vec);
    }

    add11 = kPartyItemPlayerIndex1OffsetX;
    add22 = kPartyItemPlayerIndex2OffsetX;
    add33 = kPartyItemPlayerIndex3OffsetX;
    int off;
    int idx = 0;
    off = 0;
    while (idx < ((PartyItemSelection*)self->playerSelections)->count) {
        int mode = ((PartyItemRecord*)((char*)self->playerSelections + off))->playerIndex;
        switch (mode) {
        case 0:
            break;
        case 1: {
            Layout_GetElementPosition(self->selectionLayout, idx + 4, vec);
            vec[0] += add11;
            Layout_SetElementPosition(self->selectionLayout, idx + 4, vec);
            break;
        }
        case 2: {
            Layout_GetElementPosition(self->selectionLayout, idx + 4, vec);
            vec[0] += add22;
            Layout_SetElementPosition(self->selectionLayout, idx + 4, vec);
            break;
        }
        case 3: {
            Layout_GetElementPosition(self->selectionLayout, idx + 4, vec);
            vec[0] += add33;
            Layout_SetElementPosition(self->selectionLayout, idx + 4, vec);
            break;
        }
        default:
            break;
        }
        off += sizeof(PartyItemRecord);
        ++idx;
    }

    int count = ((PartyItemSelection*)self->playerSelections)->count;
    switch (count) {
    case 1:
        break;
    case 2:
        Layout_SetElementEnabled(self->selectionLayout, 6, 0);
        Layout_SetElementEnabled(self->selectionLayout, 7, 0);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->selectionBpObject, partyItemSelectStrings.thirdPlayerGroup), 0);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->selectionBpObject, partyItemSelectStrings.fourthPlayerGroup), 0);
        BpObject_StartScriptByName(self->playerCountBpObject, partyItemSelectStrings.twoPlayerScript, 1);
        break;
    case 3:
        Layout_SetElementEnabled(self->selectionLayout, 7, 0);
        UIGroup_SetEnabled(BpObject_GetOrCreateNamedGroup(self->selectionBpObject, partyItemSelectStrings.fourthPlayerGroup), 0);
        BpObject_StartScriptByName(self->playerCountBpObject, partyItemSelectStrings.threePlayerScript, 1);
        break;
    default:
        BpObject_StartScriptByName(self->playerCountBpObject, partyItemSelectStrings.fourPlayerScript, 1);
        break;
    }
    BpObject_StartScriptByName(self->selectionBpObject, partyItemSelectStrings.selectionScript, 1);
    Layout_SetElementEnabled(self->selectionLayout, kPartyContinueElement, 0);
    IFadeMenuBase_StartMenuFadeOut(self);
}

void* Layout_GetElementValue(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void BpObject_SetEnabled(void*, int);
extern signed char s_partyItemDetailDrawFirst;
extern signed char s_inputManagerInitialized;
extern void* g_soundManager;
extern char s_gameInputManager[];
void InputManager_Dtor(void*, int);
extern float kPartyItemDialogSoundVolume;
extern float kPartyItemDialogSoundRange;
extern float kFloat_1_PartyItemSelect;
int Layout_CollectActivated(Layout*, void*, void*);
void* Layout_HitTest(void*, int, void*);
void InputManager_SetSelectedPlayer(void*, void*);
int Input_IsActionPressed(void*, int, int);
void InputManager_Init(void*);
unsigned FadeOverlay_Init(void*);
void ResourceState_Dtor(void*, int);
int FadeState_GetState(void*);
void CCharacterPreviewTable_Update(void*, float);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
// Borrow the input singleton, initializing and registering it on first use.
inline void* input897D8(char* names) {
    if (!Sing_805DC5C8()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, names);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
    return Sing_805DC5C8();
}
// Borrow the fade-state singleton used to gate selection input.
inline void* mode897D8(char* names) {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, names + 12);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    return Sing_805DC608();
}
struct List897D8 {
    int* data;
    unsigned size;
    int capacity;
    // Initialize an empty owned activation-index vector.
    List897D8() : data(0), size(0), capacity(0) {}
    // Free this owned temporary activation-index vector.
    ~List897D8() {
        if (data) {
            unsigned n = size;
            unsigned* p = &n;
            size = n - *(volatile unsigned*)p;
            appFreeUni__FPv(data);
        }
    }
};

} // extern "C"

// Show one item's four linked selection elements; itemIndex is a zero-based item ID.
template <int itemIndex> static inline void ShowPartyItemFeedback(void* self) {
    Layout_SetElementPlayerFlagsAndState(AsCPartyItemSelect(self)->selectionLayout, kPartyItemButtonBase + itemIndex, 0);
    Layout_SetElementPlayerFlagsAndState(AsCPartyItemSelect(self)->selectionLayout, kPartyItemImageBase + itemIndex, 0);
    Layout_SetElementEnabled(AsCPartyItemSelect(self)->selectionLayout, kPartyItemHighlightBase + itemIndex, (void*)1);
    Layout_SetElementEnabled(AsCPartyItemSelect(self)->selectionLayout, kPartyItemHighlightImageBase + itemIndex, (void*)1);
}

extern "C" {

// Advance detail open/close states or collect player choices and four-frame selection feedback.
// Return value: 0 = keep running; 1 = finished (selectionComplete = 1 when every player has a
// choice, 0 when cancel was pressed and the fade state allows leaving).
// Detail dialog transitions (fields on the menu object), evaluated each frame while detailRequested:
//   detailActive 0 -> 1 : release the controller script entry, hide detail elements 0..2
//   detailOpenStarted 0 -> 1 : enable detail BpObject, play open sound + open script
//   script done and no close yet : cancel or item-details action sets detailCloseRequested
//   detailCloseRequested 1, detailCloseStarted 0 -> 1 : close sound + close script
//   close script done : disable BpObject and clear all detail flags, restore input focus
// Outside the dialog: Phase 2-3 drive per-player highlight feedback, Phase 4 collects choices.
int CPartyItemSelect_UpdateSelectionAndDetail(void* self_, float deltaTime) {
    float frameDelta = deltaTime;
    register char* names = s_partyItemSelectSingletonRegistrations;
    register char* self = (char*)self_;
    Layout_Update((*(Layout**)&AsCPartyItemSelect(self)->detailLayout));
    // Phase 1: detail modal owns this frame, including open/close scripts and input focus.
    // The branch updates avatar render records and returns before normal selection polling.
    if (AsCPartyItemSelect(self)->detailRequested != 0) {
        // Refresh the modal content before advancing its opening or closing script.
        CPartyItemSelect_RefreshDetailText(self);
        if (AsCPartyItemSelect(self)->detailActive == 0) {
            void* cached = Sing_805DC5F8();
            AsCPartyItemSelect(self)->detailActive = 1;
            /* global singleton Sing_805DC5F8() */
            if (0 == cached) {
                if (s_bpCtrlHandleInitialized == 0) {
                    s_bpCtrlHandle[0] = -1;
                    s_bpCtrlHandle[1] = -1;
                    __register_global_object(s_bpCtrlHandle, (void(*)(void*,int))CBpCtrlHandle_Dtor, (names + 0x30));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            Layout_SetElementEnabled(AsCPartyItemSelect(self)->detailLayout, 0, (void*)0);
            Layout_SetElementEnabled(AsCPartyItemSelect(self)->detailLayout, 1, (void*)0);
            Layout_SetElementEnabled(AsCPartyItemSelect(self)->detailLayout, 2, (void*)0);
            s_partyItemDetailDrawFirst = 1;
        }
        if (AsCPartyItemSelect(self)->detailActive != 0) {
            if (AsCPartyItemSelect(self)->detailOpenStarted == 0) {
                BpObject_SetEnabled(AsCPartyItemSelect(self)->detailBpObject, 1);
                Sound_PlayByName(g_soundManager, partyItemSelectStrings.openSound, 0,0,0,0, kPartyItemDialogSoundVolume,kPartyItemDialogSoundRange,kFloat_1_PartyItemSelect);
                BpObject_StartScriptByName(AsCPartyItemSelect(self)->detailBpObject,partyItemSelectStrings.openDetailScript,1);
                AsCPartyItemSelect(self)->detailOpenStarted = 1;
            }
            else {
                if (BpObject_IsCurrentScriptComplete(AsCPartyItemSelect(self)->detailBpObject) != 0 && AsCPartyItemSelect(self)->detailCloseRequested == 0) {
                    {
                        /* input singleton Sing_805DC5C8() */
                        if (!Sing_805DC5C8()) {
                            if (!s_inputManagerInitialized) {
                                InputManager_Init(s_gameInputManager);
                                __register_global_object(s_gameInputManager, InputManager_Dtor, names);
                                s_inputManagerInitialized = 1;
                            }
                            Sing_805DC5C8() = s_gameInputManager;
                        }
                        // Original 0x800897D8: either input can request closure.
                        do {
                            if (Input_IsActionPressed(Sing_805DC5C8(), kActionCancel, -1) == 0) {
                                /* check action 0x14 */
                                if (0 == Sing_805DC5C8()) {
                                    if (s_inputManagerInitialized == 0) {
                                        InputManager_Init(s_gameInputManager);
                                        __register_global_object(s_gameInputManager, InputManager_Dtor, names);
                                        s_inputManagerInitialized = 1;
                                    }
                                    Sing_805DC5C8() = s_gameInputManager;
                                }
                                if (Input_IsActionPressed(Sing_805DC5C8(), kActionItemDetails, -1) == 0) break;
                            }
                            AsCPartyItemSelect(self)->detailCloseRequested = 1;
                        }
                        while (0);
                    }
                }
                else if (AsCPartyItemSelect(self)->detailCloseRequested == 1) {
                    if (AsCPartyItemSelect(self)->detailCloseStarted == 0) {
                        Sound_PlayByName(g_soundManager, partyItemSelectStrings.closeSound, 0,0,0,0, kPartyItemDialogSoundVolume,kPartyItemDialogSoundRange,kFloat_1_PartyItemSelect);
                        BpObject_StartScriptByName(AsCPartyItemSelect(self)->detailBpObject,partyItemSelectStrings.closeDetailScript,1);
                        void* cached = Sing_805DC5F8();
                        AsCPartyItemSelect(self)->detailCloseStarted = 1;
                        if (cached == 0) {
                            if (0 == s_bpCtrlHandleInitialized) {
                                s_bpCtrlHandle[0] = -1;
                                s_bpCtrlHandle[1] = -1;
                                __register_global_object(s_bpCtrlHandle, (void(*)(void*,int))CBpCtrlHandle_Dtor, (names + 0x30));
                                s_bpCtrlHandleInitialized = 1;
                            }
                            Sing_805DC5F8() = s_bpCtrlHandle;
                        }
                        CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kPartySelectScriptIndex);
                    }
                    if (BpObject_IsCurrentScriptComplete(AsCPartyItemSelect(self)->detailBpObject) != 0) {
                        BpObject_SetEnabled(AsCPartyItemSelect(self)->detailBpObject, 0);
                        AsCPartyItemSelect(self)->detailOpenStarted=0;
                        // Closing finished: clear all latches and return input focus to player zero.
                        void* cached = Sing_805DC5C8();
                        AsCPartyItemSelect(self)->detailCloseStarted=0;
                        AsCPartyItemSelect(self)->detailCloseRequested=0;
                        AsCPartyItemSelect(self)->detailActive=0;
                        if (cached == 0) {
                            if (0 == s_inputManagerInitialized) {
                                InputManager_Init(s_gameInputManager);
                                __register_global_object(s_gameInputManager, InputManager_Dtor, names);
                                s_inputManagerInitialized = 1;
                            }
                            Sing_805DC5C8() = s_gameInputManager;
                        }
                        InputManager_SetSelectedPlayer(Sing_805DC5C8(), 0);
                        AsCPartyItemSelect(self)->detailRequested=0;
                        s_partyItemDetailDrawFirst = 0;
                    }
                }
            }
        }
        CCharacterPreviewTable_Update(((RenderContext*)RenderContext_GetOrCreate())->players, frameDelta);
        return 0;
    }
    int i;
    // Phase 2: normal selection starts from hidden highlights and enabled item controls.
    Layout_Update((*(Layout**)&AsCPartyItemSelect(self)->selectionLayout));
    for (i = kPartyItemHighlightBase; i <= kPartyItemHighlightBase + kPartyItemCount - 1; ++i) {
        Layout_SetElementEnabled(AsCPartyItemSelect(self)->selectionLayout, i, (void*)0);
    }
    for (i = kPartyItemHighlightImageBase; i <= kPartyItemHighlightImageBase + kPartyItemCount - 1; ++i) {
        Layout_SetElementEnabled(AsCPartyItemSelect(self)->selectionLayout, i, (void*)0);
    }
    for (i = kPartyItemButtonBase; i <= kPartyItemButtonBase + kPartyItemCount - 1; ++i) {
        Layout_SetElementPlayerFlagsAndState(AsCPartyItemSelect(self)->selectionLayout, i, 1);
    }
    for (i = kPartyItemImageBase; i <= kPartyItemImageBase + kPartyItemCount - 1; ++i) {
        Layout_SetElementPlayerFlagsAndState(AsCPartyItemSelect(self)->selectionLayout, i, 1);
    }
    // Phase 3: PartyItemActivation::feedbackFramesRemaining means REMAINING FEEDBACK FRAMES.
    // Retain selected highlights until every player countdown reaches zero.
    // Feedback delays completion until every player\'s timer reaches zero.
    int feedbackActive = 0;
    {
        int activationOffset = 0;
        void* selectionRecords = AsCPartyItemSelect(self)->playerSelections;
        for (int i = 0; i < (((PartyItemSelection*)selectionRecords)->count); ++i) {
            if (((PartyItemActivation*)((char*)AsCPartyItemSelect(self)->playerActivations + activationOffset))->feedbackFramesRemaining != 0) {
                feedbackActive = 1;
                break;
            }
            activationOffset += kPartyActivationStride;
        }
    }
    if (feedbackActive) {
        int activationByteOffset = 0;
        int playerSlot = 0;
        // Keep selected items highlighted while their feedback timers count down.
        while (playerSlot < ((PartyItemSelection*)AsCPartyItemSelect(self)->playerSelections)->count) {
            char* activationArray = (*(char**)&AsCPartyItemSelect(self)->playerActivations);
            char* activation = (activationArray + activationByteOffset);
            if (((PartyItemActivation*)activation)->feedbackFramesRemaining != 0) {
                {
                    // Button, image, highlight and highlight-image elements share the item index.
                    switch (((PartyItemActivation*)activation)->elementId - kPartyItemButtonBase) {
                        case 0: ShowPartyItemFeedback<0>(self); break;
                        case 1: ShowPartyItemFeedback<1>(self); break;
                        case 2: ShowPartyItemFeedback<2>(self); break;
                        case 3: ShowPartyItemFeedback<3>(self); break;
                        case 4: ShowPartyItemFeedback<4>(self); break;
                        case 5: ShowPartyItemFeedback<5>(self); break;
                        case 6: ShowPartyItemFeedback<6>(self); break;
                        case 7: ShowPartyItemFeedback<7>(self); break;
                        case 8: ShowPartyItemFeedback<8>(self); break;
                        case 9: ShowPartyItemFeedback<9>(self); break;
                        case 10: ShowPartyItemFeedback<10>(self); break;
                        case 11: ShowPartyItemFeedback<11>(self); break;
                        default: break;
                    }
                }
                // One frame of selection feedback has elapsed.
                int* feedbackFrames = &((PartyItemActivation*)((*(char**)&AsCPartyItemSelect(self)->playerActivations) + activationByteOffset))->feedbackFramesRemaining;
                // 0x800893A0 unit: activation +4
                *feedbackFrames = *feedbackFrames - 1;
            }
            activationByteOffset = (activationByteOffset + kPartyActivationStride);
            playerSlot++;
        }
    }
    // Phase 4: consume parallel (elementId, playerId) activation arrays; accepted choices
    // update both the player record and its four-frame feedback. Fade states 1/3 block input.
    // Element IDs and player IDs are parallel activation arrays.
    List897D8 indices;
    List897D8 players;
    if (Layout_CollectActivated((*(Layout**)&AsCPartyItemSelect(self)->selectionLayout), &indices, &players) != 0) {
        if (FadeState_GetState(mode897D8(names)) != kFadeInputBlockedState1) {
            if (FadeState_GetState(mode897D8(names)) != kFadeInputBlockedState3) {
                for (unsigned i = 0; i < indices.size; ++i) {
                    int activatedElement = indices.data[i];
                    switch (activatedElement) {
                        default: break;
                        case kPartyItemButtonBase: case kPartyItemButtonBase + 1: case kPartyItemButtonBase + 2: case kPartyItemButtonBase + 3: case kPartyItemButtonBase + 4: case kPartyItemButtonBase + 5:
                        case kPartyItemButtonBase + 6: case kPartyItemButtonBase + 7: case kPartyItemButtonBase + 8: case kPartyItemButtonBase + 9: case kPartyItemButtonBase + 10: case kPartyItemButtonBase + 11:
                        {
                            char* selectionRecords = (*(char**)&AsCPartyItemSelect(self)->playerSelections);
                            unsigned int choice = activatedElement - kPartyItemButtonBase;
                            int selectionCount = ((PartyItemSelection*)selectionRecords)->count;
                            int selectionIndex;
                            int player = players.data[i];
                            for (selectionIndex = 0; selectionIndex < selectionCount; ++selectionIndex) {
                                if (player == ((PartyItemRecord*)(selectionRecords + selectionIndex * (int)sizeof(PartyItemRecord)))->playerIndex) break;
                            }
                            if (selectionIndex < selectionCount) {
                                if (((PartyItemActivation*)((*(char**)&AsCPartyItemSelect(self)->playerActivations) + kPartyActivationStride*selectionIndex))->feedbackFramesRemaining == 0) {
                                    ((PartyItemActivation*)((*(char**)&AsCPartyItemSelect(self)->playerActivations) + (kPartyActivationStride*player)))->elementId = (choice + kPartyItemButtonBase);
                                    ((PartyItemActivation*)((*(char**)&AsCPartyItemSelect(self)->playerActivations) + (player*kPartyActivationStride)))->feedbackFramesRemaining = kSelectionFeedbackFrames;
                                    ((PartyItemRecord*)((*(char**)&AsCPartyItemSelect(self)->playerSelections) + selectionIndex*(int)sizeof(PartyItemRecord)))->selection = choice;
                                    feedbackActive = 1;
                                    int displaySlot = -1;
                                    selectionRecords = (*(char**)&AsCPartyItemSelect(self)->playerSelections);
                                    for (int k=0; k < ((PartyItemSelection*)selectionRecords)->count; ++k) {
                                        if (player == ((PartyItemRecord*)(selectionRecords+k*(int)sizeof(PartyItemRecord)))->playerIndex) {
                                            displaySlot=k;
                                            break;
                                        }
                                    }
                                    ((*(void***)&AsCPartyItemSelect(self)->selectedItemDisplays))[displaySlot] = Layout_GetElementValue(AsCPartyItemSelect(self)->selectionLayout, choice+kPartyItemImageBase, 1);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    // Phase 5: complete only after feedback ends; otherwise accept cancel when fading permits it.
    if (!feedbackActive) {
        unsigned int complete = 1;
        char* selectionRecords = (*(char**)&AsCPartyItemSelect(self)->playerSelections);
        for (int i=0; i < ((PartyItemSelection*)selectionRecords)->count; ++i) {
            if (((PartyItemRecord*)(selectionRecords+i*(int)sizeof(PartyItemRecord)))->selection == kNoItemSelection) {
                complete=0;
                break;
            }
        }
        int pressed=0;
        for (int i=0; kPartyPlayerCount>i; ++i) pressed |= Input_IsActionPressed(input897D8(names),kActionCancel,i);
        if (complete) {
            AsCPartyItemSelect(self)->selectionComplete=1;
            return 1;
        }
        if (pressed) {
            if (FadeState_GetState(mode897D8(names)) != kFadeInputBlockedState1) {
                if (FadeState_GetState(mode897D8(names)) != kFadeInputBlockedState3) {
                    AsCPartyItemSelect(self)->selectionComplete=0;
                    return 1;
                }
            }
        }
    }
    CCharacterPreviewTable_Update(((RenderContext*)RenderContext_GetOrCreate())->players, frameDelta);
    // Phase 6: a hovered item plus the details action opens the modal for that player.
    for (i=0; i < ((PartyItemSelection*)AsCPartyItemSelect(self)->playerSelections)->count; ++i) {
        for (int selectionIndex=0; selectionIndex<kPartyItemCount; ++selectionIndex) {
            if (Layout_HitTest(AsCPartyItemSelect(self)->selectionLayout, selectionIndex+kPartyItemButtonBase,
            *(void**)&((PartyItemRecord*)((*(char**)&AsCPartyItemSelect(self)->playerSelections)+i*(int)sizeof(PartyItemRecord)))->playerIndex) != 0) {
                if (Input_IsActionPressed(input897D8(names),kActionItemDetails,((PartyItemRecord*)((*(char**)&AsCPartyItemSelect(self)->playerSelections)+i*(int)sizeof(PartyItemRecord)))->playerIndex)) {
                    AsCPartyItemSelect(self)->detailItemIndex=selectionIndex;
                    AsCPartyItemSelect(self)->detailItemDisplay=Layout_GetElementValue(AsCPartyItemSelect(self)->selectionLayout,selectionIndex+kPartyItemImageBase,1);
                    InputManager_SetSelectedPlayer(input897D8(names),*(void**)&((PartyItemRecord*)((*(char**)&AsCPartyItemSelect(self)->playerSelections)+i*(int)sizeof(PartyItemRecord)))->playerIndex);
                    AsCPartyItemSelect(self)->detailRequested=1;
                    break;
                }
            }
        }
        if (AsCPartyItemSelect(self)->detailRequested) break;
    }
    return 0;

}

} // extern "C"
