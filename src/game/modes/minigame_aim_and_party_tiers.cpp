// Mini-game aim tables and party tiers: CMiniGameAim loads the aim file and returns its
// entries, and party tier code gives each player the reward for their threshold tier.
// It also holds the new-account menu (CCreateNewAccount) and its packed resource loading.

#include <decomp/static_init.h>
#include "classes/CMiniGameAim.h" // Methods and layout owned by this unit.
#include "classes/CSharedDoc.h"
// View the existing storage as CSharedDoc without transferring ownership.
static inline CSharedDoc* AsCSharedDoc(const volatile void* p) {
    return (CSharedDoc*)p;
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CCreateNewAccount.h"

// View the existing storage as CCreateNewAccount without transferring ownership.
static inline CCreateNewAccount* AsCCreateNewAccount(const volatile void* p) {
    return (CCreateNewAccount*)p;
}

#include "iff/IffStruct.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"
#include "classes/CComStrokeLevel.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpCtrlHandle.h"
// These views fill gaps in the current shared headers without changing those headers.
// Party configuration payload: three signed-short triplets {target, comparison, reward}.
// Tier storage is low-to-high (results 1..3); aim fallback stores targets high-to-low.
// Comparison codes 4/5 mean >= / == for integer modes; distance uses code 6 (>=).
// The distance unit is not established here: thresholds are cast directly to float.
struct PartyTier {
    short target;
    short comparison;
    short reward;
};
struct PartyThresholdStorage {
    char prefix[(int)&((CSharedDoc*)0)->partyTiers];
    PartyTier tiers[3];
};
struct PartyPlayerResult {
    char prefix[0xD0];
    int reward;
    char beforeResult[0xA4];
    union {
        int integerValue;
        float distance;
    } primaryResult;
    int secondaryResult;
};
struct LoadedAimTable {
    char prefix[8];
    int count;
};
struct NewAccountMenuPool {
    char layout[16];
    char primaryAnimation[16];
    char backgroundAnimation[17];
    char captionAnimation[21];
    char textureArchive[13];
};
struct AccountRegistrations {
    char inputManager[12];
    char bpController[12];
    char reserved[12];
    char bpHandle[12];
    char resourceTable[12];
};
enum PartyGameMode {
    kPartyScore = 1,
    kPartyDistance = 2,
    kPartyCount = 3,
    kGameModePractice = 6
};
enum PartyComparison {
    kPartyAtLeast = 4,
    kPartyExactly = 5,
    kPartyDistanceAtLeast = 6
};
enum AccountOptions {
    kAccountOptionDisabled = 0x80,
    kAccountOptionEnabled = 0x40,
    kAccountPackedFileCount = 2,
    kAccountLayoutGroups = 5,
    kPartyRewardDivisor = 5
};
enum PartyFieldOffsets {
    kAccountTextureArchive = (int)&((NewAccountMenuPool*)0)->textureArchive,
    kPartyTierStorage = (int)&((CSharedDoc*)0)->partyTiers,
    kRoundNameStorage = (int)&((CSharedDoc*)0)->playNameBuffer
};

// Dispatch-only views: empty bodies preserve the original virtual slot layout.
struct VObj80326850 {
    // Placeholder for unused virtual slot 0.
    virtual void a() {}
    // Placeholder for unused virtual slot 1.
    virtual void b() {}
    // Dispatch the child dialog's draw hook through virtual slot 2.
    virtual void c() {}
    // Placeholder for unused virtual slot 3.
    virtual void d() {}
    // Placeholder for unused virtual slot 4.
    virtual void e() {}
};
struct VObj80326438 {
    // Dispatch destruction through virtual slot 0.
    virtual void a(int) {}
    // Placeholder for unused virtual slot 1.
    virtual void b(int) {}
    // Placeholder for unused virtual slot 2.
    virtual void c(int) {}
};
// Configuration +0x28 selects party mode 1/2/3; +0x2A contains the three triplets.
// +0x42/+0x44 are forwarded to course selection; their detailed meanings are unresolved.
struct AccountConfig80325A04 {
    char pad00[0x28];
    signed char mode;
    char pad29;
    short entries[9];
    char pad3c[6];
    signed char kind;
    char pad43;
    unsigned char other;
};

extern "C" {
extern char s_newAccountFadeOverlayRegistration[];

extern float kFloat_6_2831855;
extern float s_miniGameSupportTauCache1;
extern float s_miniGameSupportTauCache2;
extern float s_miniGameSupportTauCache3;
extern float s_miniGameSupportTauCache4;
extern unsigned int __vt__12CMiniGameAim[];
extern char __vt__17CCreateNewAccount[];
extern char newAccountMenuStrings[];
extern char s_newAccountInputManagerRegistration[];
extern void* Table_GetEntry(void*, int);
extern void* SharedFile_Create(void*, int);
extern int CSharedFilePtr_GetLoadedBuffer(void*);
extern void* GolfPlayer_GetEntryByIndex(void*);
extern void __dl__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void* g_sharedDoc;
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern void MPBase1_Ctor(void*);
extern void* Layout_Init(void*, int);
extern void* MenuMessage_Ctor(void*);
extern void* __nwa__FUl(unsigned int);
extern void Layout_LoadExtended(void*, int, const char*);
extern int ResourceTable_FindEntryByName(void*, const char*, unsigned int*);
extern void* s_newAccountPackedFileNames[2];
extern void Allocation_Dtor(void*);
extern void CBpCtrlHandle_Dtor(void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern void* IdMap_Lookup(void*, void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void CBpCtrl_dtor();
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_IngameStatus[];
extern signed char s_bpCtrlInitialized;
extern void Layout_Destroy(void*, int);
extern void MenuMessage_Dtor(void*, int);
extern void __dla__FPv(void*);
extern float kFloat_1_NewAccountLayoutOpacity;
extern int Layout_PollLoadComplete(void*);
extern void CourseSelection_ApplyPartyCourseTable(int, int, int, int);
extern void CourseSelection_ApplyMiniGameCourse(int, int, int);
extern void* memset(void*, int, unsigned int);

// Report that the stroke-level table is ready.
int CComStrokeLevel::vfunc_1() {
    return 1;
}
// Use the loaded aim table, or build a fallback entry from the three party thresholds.
// Resolve only after PollLoad reports ready. The loaded table is borrowed from sharedFile;
// fallbackEntry is owned here and contains three int targets in descending tier order.
void CMiniGameAim::ResolveEntries() {
    if (sharedFile) {
        table = (void*)CSharedFilePtr_GetLoadedBuffer(sharedFile);
    } else {
        if (fallbackEntry) {
            __dl__FPv(fallbackEntry);
            fallbackEntry = 0;
        }
        fallbackEntry = __nw__FUl(12);
        ((int*)fallbackEntry)[2] = ((PartyThresholdStorage*)g_sharedDoc)->tiers[0].target;
        ((int*)fallbackEntry)[1] = ((PartyThresholdStorage*)g_sharedDoc)->tiers[1].target;
        ((int*)fallbackEntry)[0] = ((PartyThresholdStorage*)g_sharedDoc)->tiers[2].target;
    }
}
// Initialize an empty mini-game aim table with no loaded file or fallback entry.
CMiniGameAim::CMiniGameAim() {
    CMiniGameAim* fields = this;
    fields->table = 0;
    fields->vtable = __vt__12CMiniGameAim;
    fields->sharedFile = 0;
    fields->fallbackEntry = 0;
}
// Initialize the first unit-local full-turn angle cache.
void MiniGameSupport_InitTauCache1() {
    s_miniGameSupportTauCache1 = kFloat_6_2831855;
}
// Initialize the second unit-local full-turn angle cache.
void MiniGameSupport_InitTauCache2() {
    s_miniGameSupportTauCache2 = kFloat_6_2831855;
}
// Report success for the first mini-game aim lifecycle phase.
int CMiniGameAim::vfunc_1() {
    return 1;
}
// Report success for the second mini-game aim lifecycle phase.
int CMiniGameAim::vfunc_2() {
    return 1;
}
// Report success for the third mini-game aim lifecycle phase.
int CMiniGameAim::vfunc_3() {
    return 1;
}
// Report success for the fourth mini-game aim lifecycle phase.
int CMiniGameAim::vfunc_4() {
    return 1;
}
// Initialize the third unit-local full-turn angle cache.
void MiniGameSupport_InitTauCache3() {
    s_miniGameSupportTauCache3 = kFloat_6_2831855;
}
// Return a loaded table entry or the single fallback entry.
// Returned memory is borrowed. In fallback mode index is ignored; ResolveEntries must
// have created the entry first, even though GetEntryCount already reports one.
void* CMiniGameAim::GetEntry(int index) {
    CMiniGameAim* fields = this;
    if (fields->sharedFile)
        return Table_GetEntry(fields->table, index);
    return fields->fallbackEntry;
}
// Return the loaded table count, zero while unresolved, or one for the fallback.
int CMiniGameAim::GetEntryCount() {
    CMiniGameAim* fields = this;
    if (fields->sharedFile) {
        if (fields->table)
            return ((LoadedAimTable*)fields->table)->count;
        return 0;
    }
    return 1;
}
// Initialize the fourth unit-local full-turn angle cache.
void MiniGameSupport_InitTauCache4() {
    s_miniGameSupportTauCache4 = kFloat_6_2831855;
}

#pragma push
#pragma use_lmw_stmw on
// Create the account layout, animations, message object, texture archive and two packed files.
CCreateNewAccount::CCreateNewAccount() {
    char* registrations;
    void** fields;
    void* resource;
    void* child;
    void* allocation;
    unsigned int resourceId;
    void* textureResource;
    int resourceFound;
    void* primaryAnimation;
    unsigned int i;
    void** packedFileNames;
    registrations = (char*)s_newAccountInputManagerRegistration;
    // Phase 1: create layout/message storage and initialize account selection flags.
    MPBase1_Ctor(this);
    fields = (void**)this;
    fields[0] = __vt__17CCreateNewAccount;
    allocation = __nw__FUl(0x5C);
    if (allocation)
        Layout_Init(allocation, kAccountLayoutGroups);
    layout = allocation;
    primaryBpObject = 0;
    fields[9] = (void*)-1;
    fields[11] = (void*)-1;
    accepted = 0;
    cancelled = 0;
    selectedCharacterIndex = 0;
    optionBits = ((optionBits & ~kAccountOptionDisabled) | kAccountOptionEnabled);
    this->child = 0;
    allocation = __nw__FUl(12);
    if (allocation) {
        try {
            MenuMessage_Ctor(allocation);
        } catch (...) {
            __dl__FPv(allocation);
            throw;
        }
    }
    menuMessage = allocation;
    packedFiles = (void**)__nwa__FUl(kAccountPackedFileCount * sizeof(void*));
    this->textureResource = 0;
    Layout_LoadExtended(layout, 0, ((NewAccountMenuPool*)newAccountMenuStrings)->layout);

    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (((AccountRegistrations*)registrations)->bpController));
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    // Phase 2: create primary/background/caption BP entries; object pointers are manager-owned.
    ((CBpCtrl*)((void* volatile&)Sing_805DC5F0()))
        ->CreateEntry(((NewAccountMenuPool*)newAccountMenuStrings)->primaryAnimation, &primaryBpId /* 0x24 */, 0);
    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (((AccountRegistrations*)registrations)->bpController));
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    primaryAnimation = IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), fields[9]);
    primaryBpObject = primaryAnimation;

    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (((AccountRegistrations*)registrations)->bpController));
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    ((CBpCtrl*)((void* volatile&)Sing_805DC5F0()))
        ->CreateEntry(((NewAccountMenuPool*)newAccountMenuStrings)->backgroundAnimation, &secondaryBpId /* 0x28 */, 0);
    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (((AccountRegistrations*)registrations)->bpController));
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    secondaryBpObject = IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), fields[10]);

    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor,
                                     (((AccountRegistrations*)registrations)->bpController));
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    ((CBpCtrl*)((void* volatile&)Sing_805DC5F0()))
        ->CreateEntry(((NewAccountMenuPool*)newAccountMenuStrings)->captionAnimation, &auxiliaryBpId /* 0x2C */, 0);

    // Phase 3: request texture archive and two packed preload files, then release the caption handle.
    resourceId = 0;
    allocation = Sing_805DC620();
    if (!allocation) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     (((AccountRegistrations*)registrations)->resourceTable));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    resourceFound =
        ResourceTable_FindEntryByName(Sing_805DC620(), newAccountMenuStrings + kAccountTextureArchive, &resourceId);
    textureResource = __nw__FUl(0x20);
    if (textureResource) {
        try {
            ((CPackTextureResource*)textureResource)
                ->Init((const char*)(newAccountMenuStrings + kAccountTextureArchive), (void**)resourceId,
                       (unsigned int)resourceFound);
        } catch (...) {
            __dl__FPv(textureResource);
            throw;
        }
    }
    this->textureResource = textureResource;
    for (i = 0, packedFileNames = s_newAccountPackedFileNames; i < kAccountPackedFileCount; ++i, ++packedFileNames)
        packedFiles[i] = CPackedFile::Create(*packedFileNames);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                     (((AccountRegistrations*)registrations)->bpHandle));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
}
#pragma pop

// Release the fallback entry and shared file handle, then optionally delete the object.
CMiniGameAim::~CMiniGameAim() {
    CMiniGameAim* fields = this;
    fields->vtable = __vt__12CMiniGameAim;
    if (fields->fallbackEntry) {
        __dl__FPv(fields->fallbackEntry);
        fields->fallbackEntry = 0;
    }
    if (fields->sharedFile) {
        CSharedFileHandle::Destroy(fields->sharedFile, 1);
        fields->sharedFile = 0;
    }
}

// Request the aim file and poll its buffer, treating practice mode or no file as ready.
int CMiniGameAim::PollLoad(int fileName) {
    unsigned int* sharedDoc = (unsigned int*)g_sharedDoc;
    CMiniGameAim* fields = this;
    if (AsCSharedDoc(sharedDoc)->gameMode /* 0x808 */ == kGameModePractice || !fileName)
        return 1;
    if (!fields->sharedFile)
        fields->sharedFile = SharedFile_Create((void*)fileName, 1);
    return CSharedFilePtr_GetLoadedBuffer(fields->sharedFile) != 0;
}

// Add the achieved party tier reward to the selected player; report whether a tier was earned.
// playerIndex is an integer index carried in a pointer-typed ABI slot, not an object pointer.
// Uses the stored partyResultTier; it does not evaluate results or prevent repeat awards.
int PartyGame_ApplyAchievedTierRewardToPlayer(void* playerIndex) {
    unsigned int* sharedDoc = (unsigned int*)g_sharedDoc;
    if (!AsCSharedDoc(sharedDoc)->partyResultTier /* 0xAF0 */)
        return 0;
    char* tierCursor = (char*)sharedDoc;
    tierCursor += (AsCSharedDoc(sharedDoc)->partyResultTier /* 0xAF0 */ - 1) * 6;
    void* playerResult = ((void* (*)(void*, void*))GolfPlayer_GetEntryByIndex)(
        (void*)AsCSharedDoc(sharedDoc)->golfDoc /* 0xB90 */, playerIndex);
    ((PartyPlayerResult*)playerResult)->reward += ((PartyThresholdStorage*)tierCursor)->tiers[0].reward;
    return 1;
}

// Divide each integer by five unless conversion is disabled by the flag.
int IntArray_DivideByFiveIfFlagClear(int* values, unsigned int count, int flag, int unused4, int unused5) {
    if (flag)
        return 0;
    for (unsigned int i = 0; i < count; ++i)
        values[i] /= kPartyRewardDivisor;
    return 1;
}

// Update callbacks and draw the account animations, layout and optional child.
void CCreateNewAccount::DrawWithCallbacks() {
    void** fields = (void**)this;
    BpObject_TryInvokeCurrentCallback(secondaryBpObject, 0);
    BpObject_Draw(secondaryBpObject);
    BpObject_TryInvokeCurrentCallback(primaryBpObject, 0);
    BpObject_Draw(primaryBpObject);
    Layout_DrawWithOpacity(layout, kFloat_1_NewAccountLayoutOpacity);
    if (child)
        ((VObj80326850*)child)->c();
    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), (*(void**)&auxiliaryBpId)), 0);
    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    BpObject_Draw(IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), (*(void**)&auxiliaryBpId)));
}

// Finish packed-file loads synchronously, poll the other resources, then release preload handles.
// The inner while loops may block; returns 0 only while texture/layout/BP state is pending.
int CCreateNewAccount::PollLoadComplete() {
    for (unsigned int i = 0; i < kAccountPackedFileCount; ++i) {
        if (packedFiles[i]) {
            while (!((CPackedFile*)(packedFiles[i]))->IsLoadFinished()) {
                ((CPackedFile*)(packedFiles[i]))->PollLoad();
            }
        }
    }
    if (textureResource && !((CPackTextureResource*)textureResource)->IsLoadFinished())
        return 0;
    if (!Layout_PollLoadComplete(layout))
        return 0;
    if (!((void* volatile&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
    }
    if (!((CBpCtrl*)(((void* volatile&)Sing_805DC5F0())))->PollAllEntriesLoaded())
        return 0;
    for (unsigned int i = 0; i < kAccountPackedFileCount; ++i) {
        void* item = packedFiles[i];
        if (item) {
            if (item)
                ((VObj80326438*)item)->a(1);
            packedFiles[i] = 0;
        }
    }
    return 1;
}

// Apply party mode, thresholds and rewards, then configure its course selection.
// keepRewardScale == 0 divides rewards by five with integer truncation toward zero.
// Returns 1 for supported mode codes. Returns 0 after clearing prior party state for other codes.
int TourPartyConfig_ApplyToSharedDoc(AccountConfig80325A04* configuration, int keepRewardScale) {
    AccountConfig80325A04* config;
    int preserveRewards;
    int zero;
    int mode;
    preserveRewards = keepRewardScale;
    config = configuration;
    zero = 0;
    (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) /* 0xAD8 */ = zero;
    memset(((PartyThresholdStorage*)g_sharedDoc)->tiers, 0, sizeof(((PartyThresholdStorage*)g_sharedDoc)->tiers));
    mode = config->mode;
    (*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier) /* 0xAF0 */ = zero;
    switch (mode) {
    case kPartyScore:
        (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) /* 0xAD8 */ = kPartyScore;
        break;
    case kPartyDistance:
        (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) /* 0xAD8 */ = kPartyDistance;
        break;
    case kPartyCount:
        (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) /* 0xAD8 */ = kPartyCount;
        break;
    default:
        return 0;
    }
    AsCSharedDoc(g_sharedDoc)->golfRuleType = 0;
    strcpy((char*)g_sharedDoc + kRoundNameStorage, ((char*)AsCSharedDoc(g_sharedDoc)->strokePlayName));
    // Copy targets/comparisons and scale rewards independently; never scale target values.
    short targets[3] = {config->entries[0], config->entries[3], config->entries[6]};
    short comparisons[3] = {config->entries[1], config->entries[4], config->entries[7]};
    int rewards[3] = {config->entries[2], config->entries[5], config->entries[8]};
    IntArray_DivideByFiveIfFlagClear(rewards, 3, preserveRewards, 0, 0);
    // Each enabled tier moves right across following disabled tiers, up to slot 2.
    // Evaluation later tests high slots first; this is not a numeric sort of targets.
    for (int i = 0; i < 3; ++i) {
        if (comparisons[i]) {
            int index = i;
            while (index < 2 && !comparisons[index + 1])
                ++index;
            ((PartyThresholdStorage*)g_sharedDoc)->tiers[index].target = targets[i];
            ((PartyThresholdStorage*)g_sharedDoc)->tiers[index].comparison = comparisons[i];
            ((PartyThresholdStorage*)g_sharedDoc)->tiers[index].reward = (short)rewards[i];
        }
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) /* 0xAD8 */ == kPartyCount)
        CourseSelection_ApplyPartyCourseTable(config->kind, 0, 1, config->other);
    else
        CourseSelection_ApplyMiniGameCourse(config->kind, 1, config->other);
    return 1;
}

#pragma opt_lifetimes off
// Return the highest party tier whose score, distance or count condition is satisfied.
// Returns 0 for no enabled condition met/unknown mode, otherwise the first tier in 3,2,1 order.
// Score/distance share the +0x178 result union; count reads the independent +0x17C word.
int PartyGame_EvaluatePlayerThresholdTier(int playerIndex) {
    char* sharedDocBytes = (char*)g_sharedDoc;
    switch ((*(int*)&AsCSharedDoc(sharedDocBytes)->partyGameMode) /* 0xAD8 */) {
    case kPartyScore: {
        // Read the integer result, then test stored tiers from 3 down to 1.
        int playerValue;
        int threshold;
        int comparisonMode;
        int satisfied;
        int tier;
        void* playerResult = ((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)(
            (void*)AsCSharedDoc(sharedDocBytes)->golfDoc /* 0xB90 */, playerIndex);
        sharedDocBytes = (char*)g_sharedDoc;
        tier = 3;
        playerValue = ((PartyPlayerResult*)playerResult)->primaryResult.integerValue;
        comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[2].comparison;
        threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[2].target;
        switch (comparisonMode) {
        case kPartyAtLeast:
            satisfied = playerValue >= threshold;
            break;
        case kPartyExactly:
            satisfied = playerValue == threshold;
            break;
        default:
            satisfied = 0;
            break;
        }
        if (!satisfied) {
            tier = 2;
            comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[1].comparison;
            threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[1].target;
            switch (comparisonMode) {
            case kPartyAtLeast:
                satisfied = playerValue >= threshold;
                break;
            case kPartyExactly:
                satisfied = playerValue == threshold;
                break;
            default:
                satisfied = 0;
                break;
            }
            if (!satisfied) {
                tier = 1;
                comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[0].comparison;
                threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[0].target;
                switch (comparisonMode) {
                case kPartyAtLeast:
                    satisfied = playerValue >= threshold;
                    break;
                case kPartyExactly:
                    satisfied = playerValue == threshold;
                    break;
                default:
                    satisfied = 0;
                    break;
                }
                if (!satisfied)
                    tier = 0;
            }
        }
        return tier;
    }
    case kPartyDistance: {
        // The same result word is a float here; comparison 6 alone enables a distance tier.
        float playerValue;
        char* thresholdBytes;
        int threshold;
        int comparisonMode;
        int satisfied;
        int tier;
        void* playerResult = ((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)(
            (void*)AsCSharedDoc(sharedDocBytes)->golfDoc /* 0xB90 */, playerIndex);
        thresholdBytes = (char*)g_sharedDoc;
        playerValue = ((PartyPlayerResult*)playerResult)->primaryResult.distance;
        for (tier = 3; tier > 0; --tier) {
            comparisonMode =
                *(short*)(thresholdBytes + (kPartyTierStorage + (int)&((PartyTier*)0)->comparison - sizeof(PartyTier)) +
                          tier * sizeof(PartyTier));
            threshold = *(short*)(thresholdBytes + (kPartyTierStorage - sizeof(PartyTier)) + tier * sizeof(PartyTier));
            switch (comparisonMode) {
            case kPartyDistanceAtLeast:
                satisfied = playerValue >= (float)threshold;
                break;
            default:
                satisfied = 0;
                break;
            }
            if (satisfied)
                break;
        }
        return tier;
    }
    case kPartyCount: {
        // Count uses the second result word and the integer comparison rules from score mode.
        int playerValue;
        int threshold;
        int comparisonMode;
        int satisfied;
        int tier;
        void* playerResult = ((void* (*)(void*, int))GolfPlayer_GetEntryByIndex)(
            (void*)AsCSharedDoc(sharedDocBytes)->golfDoc /* 0xB90 */, playerIndex);
        sharedDocBytes = (char*)g_sharedDoc;
        tier = 3;
        playerValue = ((PartyPlayerResult*)playerResult)->secondaryResult;
        comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[2].comparison;
        threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[2].target;
        switch (comparisonMode) {
        case kPartyAtLeast:
            satisfied = playerValue >= threshold;
            break;
        case kPartyExactly:
            satisfied = playerValue == threshold;
            break;
        default:
            satisfied = 0;
            break;
        }
        if (!satisfied) {
            tier = 2;
            comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[1].comparison;
            threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[1].target;
            switch (comparisonMode) {
            case kPartyAtLeast:
                satisfied = playerValue >= threshold;
                break;
            case kPartyExactly:
                satisfied = playerValue == threshold;
                break;
            default:
                satisfied = 0;
                break;
            }
            if (!satisfied) {
                tier = 1;
                comparisonMode = ((PartyThresholdStorage*)sharedDocBytes)->tiers[0].comparison;
                threshold = ((PartyThresholdStorage*)sharedDocBytes)->tiers[0].target;
                switch (comparisonMode) {
                case kPartyAtLeast:
                    satisfied = playerValue >= threshold;
                    break;
                case kPartyExactly:
                    satisfied = playerValue == threshold;
                    break;
                default:
                    satisfied = 0;
                    break;
                }
                if (!satisfied)
                    tier = 0;
            }
        }
        return tier;
    }
    }
    return 0;
}

#pragma opt_lifetimes reset
#pragma push
#pragma use_lmw_stmw on
// Release account layouts, animations, messages and packed resources, then optionally delete.
CCreateNewAccount::~CCreateNewAccount() {
    int* fields = (int*)this;
    fields[0] = (int)__vt__17CCreateNewAccount;
    if (layout) {
        Layout_Destroy((void*)layout, 1);
        layout = 0;
    }
    if (primaryBpId != -1) {
        if (!((void* volatile&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
                s_bpCtrlInitialized = 1;
            }
            ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
        }
        ((CBpCtrl*)(((void* volatile&)Sing_805DC5F0())))->DeleteEntryById(primaryBpId);
        primaryBpId = -1;
    }
    if (secondaryBpId != -1) {
        if (!((void* volatile&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
                s_bpCtrlInitialized = 1;
            }
            ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
        }
        ((CBpCtrl*)(((void* volatile&)Sing_805DC5F0())))->DeleteEntryById(secondaryBpId);
        secondaryBpId = -1;
    }
    if (child) {
        if (child)
            ((VObj80326438*)child)->a(1);
        child = 0;
    }
    if (menuMessage) {
        MenuMessage_Dtor((void*)menuMessage, 1);
        menuMessage = 0;
    }
    for (unsigned int i = 0; i < kAccountPackedFileCount; ++i) {
        if ((*(int*)&packedFiles[i])) {
            if ((*(int*)&packedFiles[i]))
                ((VObj80326438*)(*(int*)&packedFiles[i]))->a(1);
            (*(int*)&packedFiles[i]) = 0;
        }
    }
    if (packedFiles) {
        __dla__FPv((void*)packedFiles);
        packedFiles = 0;
    }
    if (textureResource) {
        if (textureResource)
            ((VObj80326438*)textureResource)->a(1);
        textureResource = 0;
    }
    if (auxiliaryBpId != -1) {
        if (!((void* volatile&)Sing_805DC5F0())) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDtorRegistration_IngameStatus);
                s_bpCtrlInitialized = 1;
            }
            ((void* volatile&)Sing_805DC5F0()) = &s_bpCtrl;
        }
        ((CBpCtrl*)(((void* volatile&)Sing_805DC5F0())))->DeleteEntryById(auxiliaryBpId);
        auxiliaryBpId = -1;
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462608", _ctors_u80325650_0, MiniGameSupport_InitTauCache1);
STATIC_INIT_ENTRY(".CRT$XCU80462608", _ctors_u80325650_1, MiniGameSupport_InitTauCache2);
STATIC_INIT_ENTRY(".CRT$XCU80462608", _ctors_u80325650_2, MiniGameSupport_InitTauCache3);
STATIC_INIT_ENTRY(".CRT$XCU80462608", _ctors_u80325650_3, MiniGameSupport_InitTauCache4);
#pragma pop
