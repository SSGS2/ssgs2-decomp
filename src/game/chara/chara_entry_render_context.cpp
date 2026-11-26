// Render context for the character select entry screen: creating it, advancing its layout and
// texture loaders, and reporting when its resources are ready. Also the player entry state cycle.

#include "classes/CEnemyRenderTable.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "game/vector.h"

#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
#include "classes/CCharaEntry.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrl.h"
extern "C" {

void RenderContext_ResetPlayerRecords(void*);
// CharacterPreviewTable_dtor: teardown of the render record table at +0x20D0; the int is passed as -1 here (meaning not established).
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

// Layout of the render-context singleton (s_renderContext), derived from the init and rollback code:
//   0x0000: 4 save-data player records, 0x828 bytes each (SaveData_InitPlayerRecord)
//   0x20A0: IntVectorArray header area, 4 x 12-byte vectors (rollback frees each)
//   0x20D0: render record table, 4 records of 0xA4 bytes (CEnemyRenderTable-typed here, count at its start)
enum RenderContextLayout {
    kSavePlayerRecordSize = 0x828,
    kVectorArrayOffset = 0x20A0,
    kVectorSize = 12,
    kVectorCount = 4,
    kRecordTableOffset = 0x20D0,
    kRenderRecordSize = 0xA4,
    kRenderRecordCount = 4,
    kLayoutHandleOffset = 0x2C // CCharaEntry field holding the borrowed layout object
};

struct RenderContextConstructionCleanup_u8024EAD8 {
    char* base;
    bool active;
    // Arm rollback for the already constructed render-context subobjects.
    explicit RenderContextConstructionCleanup_u8024EAD8(void* object) : base((char*)object), active(true) {}
    // Release the record array and ID allocations if reset did not complete.
    ~RenderContextConstructionCleanup_u8024EAD8() {
        if (active) {
            CharacterPreviewTable_dtor(base + kRecordTableOffset, -1);
            for (int index = kVectorCount - 1; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + kVectorArrayOffset + index * kVectorSize);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset the context and disarm construction rollback only after success.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u8024EAD8 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void __dt__13RenderContextFv(void*);
void __construct_array(void*, void*, void*, int, int);
void RenderContext_ResetPlayerRecords(void*);
void Layout_SetElementEnabled(void*, void*, int);
void Layout_SetElementPlayerFlag(void*, void*, int, int);

struct RenderContextView;
extern signed char s_renderContextInitialized;
extern char s_renderContext[];
extern char s_renderContextDtorRegistration_80506F1C[];

void Layout_Update(void*);
void CBpCtrl_dtor(void*);
void __register_global_object(void*, void*, void*);
int Layout_PollLoadComplete(void*);

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_80506F04[];

// Advance the borrowed layout and texture loader; readiness is queried separately.
void CCharaEntry::UpdateResourceLoaders() {
    void** fields = (void**)this;
    Layout_Update(fields[0x2C / 4]);
    ((CPackTextureResource*)(fields[0x3C / 4]))->PollLoad();
}

// Return nonzero only when textures, all controller entries, and layout are ready.
// +0x2C is the layout, +0x3C the pack loader; this hook polls BP/layout readiness.
// The update hook above must keep advancing the pack loader while this returns zero.
int CCharaEntry::PollResourcesReady() {
    void** fields = (void**)this;
    // This checks the texture archive's pending path; PollLoad clears it after loading.
    if (((CPackTextureResource*)(fields[0x3C / 4]))->IsLoadFinished()) {
        if (!((int&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        if (((CBpCtrl*)(((int&)Sing_805DC5F0())))->PollAllEntriesLoaded())
            return Layout_PollLoadComplete(fields[0x2C / 4]);
    }
    return 0;
}

// Local bitfield view of one 0xA4-byte render record, not a separate allocation.
// x18 is passed as player-flag channel 1's value. b0/b1 are untouched here.
// b2/b3 select the three presentations below; their game-facing labels are unknown.
struct CharacterPreviewRecordView {
    char pad0[0x18];
    int x18;
    char pad1[2];
    CW_BITS5(unsigned short b0 : 1, unsigned short b1 : 1, unsigned short b2 : 1, unsigned short b3 : 1,
             unsigned short rest : 12)
    char pad2[0xA4 - 0x20];
};

struct RenderContextView {
    char pad0[0x20D0];
    CharacterPreviewRecordView entries[4];
};
// Presentation names describe the resulting b2/b3 bits, not unverified UI labels.
enum PlayerEntryLayoutIds {
    kPresentation00Base = 0x10,
    kPresentation01Base = 0x14,
    kPresentation10Base = 0x18,
    kPresentation00AuxBase = 0x3C,
    kPresentation01AuxBase = 0x34,
    kPresentation10AuxBase = 0x38,
    kEntryInteractionFlagChannel = 0,
    kEntryPlayerValueChannel = 1,
    kEntryInteractionFlagValue = 5
};

// Return the borrowed singleton, constructing its four saves/vectors/records once.
static inline RenderContextView* GetMgr8024EAD8() {
    if (!((RenderContextView*&)Sing_805DC648())) {
        if (!s_renderContextInitialized) {
            char* base = s_renderContext;
            char* p = base;
            do {
                SaveData_InitPlayerRecord(p);
                p += kSavePlayerRecordSize;
            } while (p < base + kVectorArrayOffset);
            ((IntVectorArray*)(base + kVectorArrayOffset))->__ct();
            char* q = base + kRecordTableOffset;
            __construct_array(q, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, kRenderRecordSize,
                              kRenderRecordCount);
            AsCEnemyRenderTable(q)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDtorRegistration_80506F1C);
            s_renderContextInitialized = 1;
        }
        ((RenderContextView*&)Sing_805DC648()) = (RenderContextView*)s_renderContext;
    }
    return ((RenderContextView*&)Sing_805DC648());
}

#pragma use_lmw_stmw on
// Cycle one player's presentation and enable the matching layout elements.
// idx must be 0..3. Layout IDs are integer values carried in pointer arguments:
// base+idx, never dereferenced. b2/b3 transition and enabled element map:
//   10 -> 00: 0x10+idx and 0x3C+idx
//   01 -> 10: 0x18+idx and 0x38+idx
//   00 -> 01: 0x14+idx and 0x34+idx
// If both bits are set, b2 wins. Each branch disables the other two main elements;
// auxiliary elements are only enabled here, so their previous state is retained.
// Flag channel 0 gets value 5; channel 1 gets record.x18. Names remain unresolved.
void CCharaEntry::CyclePlayerEntryState(int idx) {
    unsigned char* object = (unsigned char*)this;
    unsigned char* encodedPlayerId = (unsigned char*)idx;
    if (GetMgr8024EAD8()->entries[idx].b2) {
        GetMgr8024EAD8()->entries[idx].b2 = 0;
        GetMgr8024EAD8()->entries[idx].b3 = 0;
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00Base, 1);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01Base, 0);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10Base, 0);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00Base,
                                    kEntryInteractionFlagChannel, kEntryInteractionFlagValue);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00Base,
                                    kEntryPlayerValueChannel, GetMgr8024EAD8()->entries[idx].x18);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00AuxBase, 1);
    } else if (GetMgr8024EAD8()->entries[idx].b3) {
        GetMgr8024EAD8()->entries[idx].b2 = 1;
        GetMgr8024EAD8()->entries[idx].b3 = 0;
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00Base, 0);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01Base, 0);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10Base, 1);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10Base,
                                    kEntryInteractionFlagChannel, kEntryInteractionFlagValue);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10Base,
                                    kEntryPlayerValueChannel, GetMgr8024EAD8()->entries[idx].x18);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10AuxBase, 1);
    } else {
        GetMgr8024EAD8()->entries[idx].b2 = 0;
        GetMgr8024EAD8()->entries[idx].b3 = 1;
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation00Base, 0);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01Base, 1);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation10Base, 0);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01Base,
                                    kEntryInteractionFlagChannel, kEntryInteractionFlagValue);
        Layout_SetElementPlayerFlag(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01Base,
                                    kEntryPlayerValueChannel, GetMgr8024EAD8()->entries[idx].x18);
        Layout_SetElementEnabled(*(void**)(object + kLayoutHandleOffset), encodedPlayerId + kPresentation01AuxBase, 1);
    }
}
#pragma use_lmw_stmw reset

} // extern "C"
