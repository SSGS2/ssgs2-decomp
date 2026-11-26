// Character select entries (CCharaEntry): construction and destruction of an entry, its selection
// storage, its scripts and UI resources, and the BP controller it shares.
// The entry loads its archive and registers the three entry scripts when it is built.

#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;

#include "game/singletons.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CSingleTaskRev.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CCharaEntry.h"
#include "classes/CBpCtrl.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpCtrlHandle.h"
extern "C" {

void __dt__14CSingleTaskRevFv(void*, int);
void MPBase1_Ctor(void*);
int ResourceTable_FindEntryByName(void*, const char*, int*);
void Layout_Init(void*, int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void Layout_SetActive(void*, int);
void CBpCtrl_dtor(void*);
void Allocation_Dtor(void*);
void* __nw__FUl(unsigned int);
void __register_global_object(void*, void*, void*);
void* __nwa__FUl(unsigned int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void Layout_Destroy(void*, int);
void __dla__FPv(void*);
void* RenderContext_GetOrCreate(void);
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
void CSingleTaskRev::vfunc_1_ThunkMinus8(int forwarded1) {
    __dt__14CSingleTaskRevFv((char*)this - 8, forwarded1);
}

// Publish the BP controller once; the global lifetime registry owns its storage.
static inline void EnsureCharacterEntryBpController128() {
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
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
CCharaEntry::~CCharaEntry() {
    Obj80249* self = (Obj80249*)this;
    int entryScriptId = self->entryScriptId;
    self->vtable = __vt__11CCharaEntry;
    if (entryScriptId != kNoScript) {
        EnsureCharacterEntryBpController128();
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(self->entryScriptId);
        self->entryScriptId = kNoScript;
    }
    if (self->noEntryScriptId != kNoScript) {
        EnsureCharacterEntryBpController128();
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(self->noEntryScriptId);
        self->noEntryScriptId = kNoScript;
    }
    if (self->characterStatsScriptId != kNoScript) {
        EnsureCharacterEntryBpController128();
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById(self->characterStatsScriptId);
        self->characterStatsScriptId = kNoScript;
    }
    CCharacterPreviewTable::SetScriptEnabled(
        &((CharacterEntryRenderContext*)RenderContext_GetOrCreate())->enemyRenderTable, 0);
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
    ((CBpCtrlHandle*)(((int&)Sing_805DC5F8())))->ReleaseEntry();
    if (self->shop != 0) {
        VObj* shop = self->shop;
        if (shop != 0) {
            shop->Destroy(1);
        }
        self->shop = 0;
    }
    ReleaseCharacterSelectionStorage128(self);
}

// Release the selection vector storage and optionally delete its container.
void* dtor_80249F28(MSLPointerVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

} // extern "C"

// Load the character-entry archive, construct its layout and register the three entry scripts.
CCharaEntry::CCharaEntry() {
    MPBase1_Ctor(this);
    vtbl = __vt__11CCharaEntry;
    result = 0;
    layout = 0;
    shopState = 0;
    entryScriptId = (void*)kNoScript;
    noEntryScriptId = (void*)kNoScript;
    characterStatsScriptId = (void*)kNoScript;
    selectionStorage = 0;
    selectionCount = 0;
    selectionCapacity = 0;
    shop = 0;
    shopOpen = 0;
    int archiveLookupInfo = 0;
    if (((volatile int&)Sing_805DC620()) == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, Allocation_Dtor,
                                     s_resourceTableDtorRegistration_CharacterEntry);
            s_resourceTableInitialized = 1;
        }
        ((volatile int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    // The two allocation pointers are register locals: that is how the original assigns their registers.
    register void* allocation;
    register void* layoutObject;
    register int archiveLookupResult = ResourceTable_FindEntryByName((void*)((volatile int&)Sing_805DC620()),
                                                                     characterEntryStrings, &archiveLookupInfo);
    allocation = __nw__FUl(kTextureResourceBytes);
    if (allocation) {
        ((CPackTextureResource*)allocation)
            ->Init((const char*)characterEntryStrings, (void**)archiveLookupInfo, (unsigned int)archiveLookupResult);
    }
    textureResource = allocation;
    ready[0] = 0;
    ready[1] = 0;
    ready[2] = 0;
    ready[3] = 0;
    layoutObject = __nw__FUl(kLayoutObjectBytes);
    if (layoutObject)
        Layout_Init(layoutObject, kEntryLayoutMode);
    layout = layoutObject;
    Layout_InitStandard(layoutObject, 0, s_characterEntryLayoutResourcePrefix, kEntryLayoutElementCount,
                        ((CharacterEntryStringPool*)characterEntryStrings)->layout);
    Layout_SetActive(layout, 1);
    EnsureCharacterEntryBpController128();
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry(((CharacterEntryStringPool*)characterEntryStrings)->entryScript, &entryScriptId, 0);
    EnsureCharacterEntryBpController128();
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry(((CharacterEntryStringPool*)characterEntryStrings)->noEntryScript, &noEntryScriptId, 0);
    EnsureCharacterEntryBpController128();
    ((CBpCtrl*)(((int&)Sing_805DC5F0())))
        ->CreateEntry(((CharacterEntryStringPool*)characterEntryStrings)->statsScript, &characterStatsScriptId, 0);
    statBarTransitions = (int*)__nwa__FUl(kStatBarTransitionBytes);
    readyFrames = 0;
    selectedPlayerIndex = kNoScript;
    started = 0;
}
