#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
// Translation unit 0x80249C48-0x8024A284 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CEnemyRenderTable.h"
extern "C" {

void CSingleTaskRev_dtor(void*, int);
void MPBase1_Ctor(void*);
int ResourceTable_FindEntryByName(void*, const char*, int*);
void CPackTextureResource_Init(void*, const char*, int, int);
void Layout_Init(void*, int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void Layout_SetActive(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void Allocation_Dtor(void*);
void* __nw__FUl(unsigned int);
void __register_global_object(void*, void*, void*);
void CBpCtrl_CreateEntry(void*, const char*, int*, int);
void* __nwa__FUl(unsigned int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void CBpCtrl_DeleteEntryById(void*, int);
void Layout_Destroy(void*, int);
void __dla__FPv(void*);
void* RenderContext_GetOrCreate(void);
void CCharacterPreviewTable_SetScriptEnabled(void*, int);
void CBpCtrlHandle_ReleaseEntry(void*);
void CBpCtrlHandle_Dtor(void*);

extern signed char s_resourceTableInitialized;
extern int s_resourceTableStorage;
extern signed char s_bpCtrlInitialized;
extern char __vt__11CCharaEntry[];
extern char characterEntryStrings[];
struct CharacterEntryStringPool {
    char archive[0x10];
    char layout[0xA];
    char entryScript[0xA];
    char noEntryScript[0x15];
    char statsScript[0x16];
};
enum {
    kNoScript = -1,
    kEntryLayoutMode = 5,
    kEntryLayoutElementCount = 0x6C,
    kTextureResourceBytes = 0x20,
    kLayoutObjectBytes = 0x5C,
    kStatBarTransitionBytes = 0x30
};
extern char s_characterEntryLayoutResourcePrefix[]; // Resource-name prefix passed to the character-entry layout loader.
extern char s_resourceTableDtorRegistration_CharacterEntry[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_80506F04[];
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleDtorRegistration_CharacterEntry[];

struct VObj {
    // First callable slot in both resource/shop vtables is a deleting destructor.
    virtual void Destroy(int deleteFlag);
};
struct CharacterEntryRenderContext {
    char renderContextPrefix[0x20D0];
    CEnemyRenderTable enemyRenderTable;
};
struct SelectionStorageView {
    void* data;
    volatile int count;
    unsigned int capacity;
};
struct Obj80249 {
    void* vtable;
    char taskBaseStorage[0x14];
    // Four MPBase1 words are cleared after texture construction; their consumers are unresolved.
    void* basePointer18;
    void* basePointer1c;
    void* basePointer20;
    void* basePointer24;
    int initialEntryState;
    void* layout;
    void* statBarTransitions;
    int statBarState;
    int selectedPlayerIndex;
    VObj* textureResource;
    void* entryContext;
    int entryScriptId;
    int noEntryScriptId;
    int characterStatsScriptId;
    int initialSelectionState;
    void* selectionStorage;
    volatile int selectionCount;
    void* selectionCapacity; // ABI word: element capacity, not an address.
    VObj* shop;
    signed char shopOpen;
};

// Adjust the secondary task-interface pointer before forwarding the virtual call.
void CSingleTaskRev_vfunc_1_ThunkMinus8(void* self, int forwarded1) { CSingleTaskRev_dtor((char*)self - 8, forwarded1); }

// Publish the BP controller once; the global lifetime registry owns its storage.
static inline void EnsureCharacterEntryBpController128() {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

// Release the owned selection-vector allocation; five guards preserve its expanded destructor ABI.
static inline void ReleaseCharacterSelectionStorage128(Obj80249* self) {
    if (&self->selectionStorage != 0) {
        if (&self->selectionStorage != 0) {
            if (&self->selectionStorage != 0) {
                if (&self->selectionStorage != 0) {
                    if (&self->selectionStorage != 0) {
                        if (self->selectionStorage != 0) {
                            // The volatile self-subtraction clears count while retaining the matched load schedule.
                            register int previousCount = self->selectionCount;
                            self->selectionCount = previousCount - *(volatile int*)&previousCount;
                            appFreeUni__FPv(self->selectionStorage);
                        }
                    }
                }
            }
        }
    }
}

// Release scripts and owned UI resources, then destroy the selection vector and optional allocation.
void* CCharaEntry_dtor(Obj80249* self, int shouldDelete) {
    if (self != 0) {
        int entryScriptId = self->entryScriptId;
        self->vtable = __vt__11CCharaEntry;
        if (entryScriptId != kNoScript) {
            EnsureCharacterEntryBpController128();
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->entryScriptId);
            self->entryScriptId = kNoScript;
        }
        if (self->noEntryScriptId != kNoScript) {
            EnsureCharacterEntryBpController128();
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->noEntryScriptId);
            self->noEntryScriptId = kNoScript;
        }
        if (self->characterStatsScriptId != kNoScript) {
            EnsureCharacterEntryBpController128();
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->characterStatsScriptId);
            self->characterStatsScriptId = kNoScript;
        }
        CCharacterPreviewTable_SetScriptEnabled(&((CharacterEntryRenderContext*)RenderContext_GetOrCreate())->enemyRenderTable, 0);
        if (self->layout != 0) {
            Layout_Destroy(self->layout, 1);
            self->layout = 0;
        }
        if (self->statBarTransitions != 0) {
            __dla__FPv(self->statBarTransitions);
            self->statBarTransitions = 0;
        }
        if (self->textureResource != 0) {
            VObj* textureResource = self->textureResource;
            if (textureResource != 0) {
                textureResource->Destroy(1);
            }
            self->textureResource = 0;
        }
        if (((int&)Sing_805DC5F8()) == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = kNoScript;
                s_bpCtrlHandle[1] = kNoScript;
                __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_CharacterEntry);
                s_bpCtrlHandleInitialized = 1;
            }
            ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)((int&)Sing_805DC5F8()));
        if (self->shop != 0) {
            VObj* shop = self->shop;
            if (shop != 0) {
                shop->Destroy(1);
            }
            self->shop = 0;
        }
        ReleaseCharacterSelectionStorage128(self);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Release the selection vector storage and optionally delete its container.
void* dtor_80249F28(MSLPointerVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Load the character-entry archive, construct its layout and register the three entry scripts.
void CCharaEntry_Ctor(Obj80249* self) {
    MPBase1_Ctor(self);
    int resourceTableInstance = ((volatile int&)Sing_805DC620());
    self->vtable = __vt__11CCharaEntry;
    self->initialEntryState = 0;
    self->layout = 0;
    self->entryContext = 0;
    self->entryScriptId = kNoScript;
    self->noEntryScriptId = kNoScript;
    self->characterStatsScriptId = kNoScript;
    self->selectionStorage = 0;
    self->selectionCount = 0;
    self->selectionCapacity = 0;
    self->shop = 0;
    self->shopOpen = 0;
    int archiveLookupInfo = 0;
    if (resourceTableInstance == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, Allocation_Dtor, s_resourceTableDtorRegistration_CharacterEntry);
            s_resourceTableInitialized = 1;
        }
        ((volatile int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    register int archiveLookupResult = ResourceTable_FindEntryByName((void*)((volatile int&)Sing_805DC620()), characterEntryStrings, &archiveLookupInfo);
    register void* allocation = __nw__FUl(kTextureResourceBytes);
    if (allocation) {
        try {
            CPackTextureResource_Init(allocation, characterEntryStrings, archiveLookupInfo, archiveLookupResult);
        } catch (...) {
            __dl__FPv(allocation);
            throw;
        }
    }
    self->textureResource = (VObj*)allocation;
    self->basePointer18 = 0;
    self->basePointer1c = 0;
    self->basePointer20 = 0;
    self->basePointer24 = 0;
    allocation = __nw__FUl(kLayoutObjectBytes);
    if (allocation) Layout_Init(allocation, kEntryLayoutMode);
    self->layout = allocation;
    Layout_InitStandard(allocation, 0, s_characterEntryLayoutResourcePrefix, kEntryLayoutElementCount, ((CharacterEntryStringPool*)characterEntryStrings)->layout);
    Layout_SetActive(self->layout, 1);
    EnsureCharacterEntryBpController128();
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), ((CharacterEntryStringPool*)characterEntryStrings)->entryScript, &self->entryScriptId, 0);
    EnsureCharacterEntryBpController128();
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), ((CharacterEntryStringPool*)characterEntryStrings)->noEntryScript, &self->noEntryScriptId, 0);
    EnsureCharacterEntryBpController128();
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), ((CharacterEntryStringPool*)characterEntryStrings)->statsScript, &self->characterStatsScriptId, 0);
    self->statBarTransitions = __nwa__FUl(kStatBarTransitionBytes);
    self->statBarState = 0;
    self->selectedPlayerIndex = kNoScript;
    self->initialSelectionState = 0;
}

} // extern "C"
