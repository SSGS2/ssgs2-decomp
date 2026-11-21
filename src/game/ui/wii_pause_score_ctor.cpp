extern "C" {
extern unsigned char s_scorecardOptionsLayoutDescriptors[]; // Fixed scorecard option-layout descriptors.
extern unsigned char s_scorecardAlternatePlayerLabelLayoutDescriptors[]; // Two-player alternate-label descriptors.
extern unsigned char s_scorecardPlayerLabelLayoutDescriptors[]; // Scorecard player-label descriptors.
extern unsigned char s_scorecardTieMarkerLayoutDescriptors[]; // Scorecard tie-marker descriptors.
extern unsigned char s_scorecardLossMarkerLayoutDescriptors[]; // Scorecard loss-marker descriptors.
extern unsigned char s_scorecardPlayerCountTables[]; // Player-count layout tables and graph presets.
}
// Translation unit 0x8032D0FC-0x8032E804 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CWiiPauseScore.h"

#define AsCWiiPauseScore(p) ((CWiiPauseScore*)(p))
// Use caller-provided storage for zero-value record construction.
inline void* operator new(size_t, void* p) { return p; }

// Both record types are zero-filled; this unit does not establish their word semantics.
struct PairValue { unsigned int a,b; };
struct TripleValue { unsigned int a,b,c; };

extern "C" {

// ABI storage view: use CWiiPauseScore's existing names for semantic fields below.
// aD8/aDC/aE0 is an owning {holeInfo, size, capacity} vector of 8-byte pairs;
// aE4/aE8/aEC is {playerHoleScores, size, capacity} of 12-byte triples.
// Pair/triple component meanings are not established by this constructor.
// ids[0..3] select common score groups; [4..7] and [8..11] are per-player
// group sets. IDs are local to p2_score_N.b2d, not global UI element numbers.
struct U8032D0FC {
    void* vtable; int pad04[5]; int a18,a1c,a20,a24,a28,a2c;
    void* ptr30; void* ptr34; void* ptr38; void* ptr3c; void* ptr40; void* ptr44;
    int a48,a4c,a50,a54; unsigned char playerCount,holeCount; char pad5a[2]; void* arg5c; int a60;
    int ids[20];              // +64 script-local group selectors; named slots below
    char padB4[0x24];
    void* aD8;                // +D8 owned holeInfo pair-record buffer
    unsigned int aDC;         // +DC holeInfo logical record count
    unsigned int aE0;         // +E0 holeInfo allocation capacity, in pairs
    void* aE4;                // +E4 owned playerHoleScores triple-record buffer
    unsigned int aE8;         // +E8 playerHoleScores logical record count
    unsigned int aEC;         // +EC playerHoleScores capacity, in triples
    int enabled[10];
};
// Script-local IDs differ in p2_score_1..4.b2d. The arrays retain their ABI
// names; these slots group controls by purpose confirmed by the shared layout.
// The precise role of the four common groups is not established in this unit.
enum PauseScoreGroupSlot {
    kCommonScoreGroup0 = 0, kCommonScoreGroup1 = 1,
    kCommonScoreGroup2 = 2, kCommonScoreGroup3 = 3,
    kPlayer0Group = 4, kPlayer1Group = 5, kPlayer2Group = 6, kPlayer3Group = 7,
    kPlayer0AlternateGroup = 8, kPlayer1AlternateGroup = 9,
    kPlayer2AlternateGroup = 10, kPlayer3AlternateGroup = 11
};
// The constructor establishes layout behavior for these mode codes; gameplay
// names of the other modes (2/3/5) are unresolved and their values stay explicit.
enum PauseScoreMode {
    kScoreModeSuppressCard = 0, kScoreModeAlternateGroups = 1,
    kScoreModeFixedTables = 4, kScoreModeSkipLayouts = 6
};
static const unsigned int kMaxHoleRecords = 0x1FFFFFFFU;
static const unsigned int kHoleRecordSmallGrowthLimit = 0x0AAAAAAAU;
static const unsigned int kHoleRecordHalfGrowthLimit = 0x15555554U;
static const unsigned int kMaxPlayerHoleRecords = 0x15555555U;
static const unsigned int kPlayerHoleSmallGrowthLimit = 0x071C71C7U;
static const unsigned int kPlayerHoleHalfGrowthLimit = 0x0E38E38EU;
enum PauseScoreLayoutConstants { kScoreLayoutKind = 5, kMenuControlCount = 10 };
// IDs in each script are intentionally kept beside the corresponding player
// count. They identify resource groups, not hole numbers or global control IDs.
extern void MPBase1_Ctor(void*);
extern void CBpCtrl_CreateEntry(void*, const char*, int*, int);
extern void* IdMap_Lookup(void*, int);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*, int);
extern signed char s_bpCtrlInitialized;
extern char __vt__14CWiiPauseScore[], s_bpCtrl[], wiiPauseScoreStrings[];
extern char pauseScorePartTotalGroupName[], s_pauseBpCtrlRegistration[];
extern void* __nw__FUl(unsigned int);
extern void Layout_Init(void*, int);
extern void Layout_InitStandard(void*, int, const void*, unsigned int, const void*);
extern void Layout_LoadExtended(void*, int, const char*);
extern void __dl__FPv(void*);
extern void PointerVector_InitReserved(void*);
extern void* CFontNumDraw_CreateInVector(void*);
extern void PointerVector_DestroyStorage(void*, int);
extern signed char s_pointerVectorInitialized;
extern char s_fontNumDrawRegistry[], s_pauseFontRegistryRegistration[];
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*, int);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void dtor_80333C6C(void*, int);
extern void dtor_80333BF0(void*, int);
extern void __dt__16CWarningLogoBaseFv(void*, int);
struct ConstructionGuard {
    U8032D0FC* object;
    // Arm destruction of already-constructed member vectors and the base on unwind.
    ConstructionGuard(U8032D0FC* p): object(p) {}
    // Release member containers in reverse construction order if construction fails.
    ~ConstructionGuard() {
        if(object) {
            dtor_80333C6C(&object->aE4,-1);
            dtor_80333BF0(&object->aD8,-1);
            __dt__16CWarningLogoBaseFv(object,0);
        }
    }
};
struct ArrayTransaction {
    void* data;
    unsigned int size, capacity;
    unsigned int* originalCapacity;
    unsigned int remaining;
    // Stage an allocation before committing vector pointer/count/capacity.
    ArrayTransaction(unsigned int* c):data(0),size(0),capacity(0),originalCapacity(c),remaining(0) {}
    // Free whichever allocation remains in the transaction after commit or unwind.
    ~ArrayTransaction() { if(data) { size=size-size; appFreeUni__FPv(data); } }
};
struct TextGuard {
    char* data;
    void* allocator;
    // Guard exception-message bytes until the shared deleter takes ownership.
    TextGuard(char* p, void* a):data(p),allocator(a) {}
    // Delete the message array if ownership has not transferred.
    ~TextGuard() { if(data) __dla__FPv(data); }
};
struct ErrorGuard_u8032D0FC {
    void* object;
    // Track a partially built exception base until the throw object is complete.
    ErrorGuard_u8032D0FC(void* p):object(p) {}
    // Destroy the exception base on a construction failure.
    ~ErrorGuard_u8032D0FC() { if(object) __dt__Q23std9exceptionFv(object,0); }
};
struct LayoutResource {
    char data[0x5c];
    // Initialize the owned layout wrapper with its original layout kind.
    LayoutResource(int n) { Layout_Init(this,n); }
};

// The two arrays use the same grow-and-insert algorithm with different values.
struct ErrorText { void* vtable; char* text; void* owner; };
struct TextOwner { void* vtable; unsigned int refs, count; char* text; };
// Build the length exception in the caller scope so MWCC keeps its unwind layout.
// TextGuard owns message bytes until TextOwner takes them; ErrorGuard owns the
// exception base until the length_error vtable/throw object is complete.
// Keep exception construction in the caller's original unwind scope. An inline
// builder changes .text and exception tables; errorPtr is an explicit output
// parameter naming that caller's throw storage, not an implicit macro dependency.
#define LENGTH_ERROR(errorPtr) do { \
    ErrorText error; \
    errorPtr=&error; \
    const char* message=wiiPauseScoreStrings+0x96; \
    error.vtable=__vt__Q23std11logic_error; \
    { \
    ErrorGuard_u8032D0FC baseGuard(errorPtr); \
    char allocator=0; \
    unsigned int length=strlen(message)+1; \
    error.text=(char*)__nwa__FUl((length<<1)-length); \
    { \
        char* text=error.text; \
        TextGuard textGuard(text,&allocator); \
        TextOwner* owner=(TextOwner*)__nw__FUl(0x10); \
        if(owner) { owner->refs=1; owner->count=1; owner->vtable=s_sharedCharDeleterVtable; owner->text=text; } \
        error.owner=owner; \
        textGuard.data=0; \
    } \
    strcpy(error.text,message); \
    baseGuard.object=0; \
    } \
    errorPtr->vtable=__vt__Q23std12length_error; \
    __throw(wiiPauseScoreStrings+0xaa,errorPtr,(void*)__dt__Q23std12length_errorFv); \
} while(0)

// Reset logical size, then fill REQUEST zero values. Reuse spare capacity or stage
// a new allocation: construct the appended range, copy old records backwards, swap
// ownership, free old storage. Growth is 60%, then 50%, then MAXIMUM. The macro
// keeps VALUE placement construction and caller lvalues in the original EH scope;
// changing that scope requires comparing exception tables as well as instructions.
#define RESIZE_VALUES(VALUE, DATA, SIZE, CAPACITY, REQUEST, MAXIMUM, LOW, HIGH) do { \
    register ErrorText* errorPtr; \
    register unsigned int oldCapacity; \
    register unsigned int amount; \
    /* Phase 1: clear logical elements and compute the requested zero-fill span. */ \
    unsigned int oldSize=*(volatile unsigned int*)&(SIZE); \
    unsigned int wanted=(REQUEST); \
    (SIZE)=(SIZE)-oldSize; \
    unsigned int size=(SIZE); \
    if(wanted>size) { \
        amount=wanted-size; \
        unsigned int capacity=(CAPACITY); \
        if(amount>capacity || size>capacity-amount) { \
            oldCapacity=*(volatile unsigned int*)&(CAPACITY); \
            if(*(volatile unsigned int*)&(SIZE)+amount-capacity>(MAXIMUM)-oldCapacity) { LENGTH_ERROR(errorPtr); } \
            /* 0x8032D0FC: capacity tiers share the growth preflight exit. */ \
            do { \
                if(oldCapacity<(LOW)) { break; } else if(oldCapacity<(HIGH)) { break; } \
            } while(0); \
        } else { \
            VALUE* out=(VALUE*)(DATA)+size; \
            while(amount!=0) { \
                VALUE value={0}; \
                new(out) VALUE(value); \
                --amount; ++out; ++(SIZE); \
            } \
            break; \
        } \
        { \
            /* Phase 2: stage replacement allocation under its transaction guard. */ \
            unsigned int extra=(SIZE)+amount-(CAPACITY); \
            ArrayTransaction transaction(&(CAPACITY)); \
            unsigned int current=(CAPACITY); \
            if(extra>(MAXIMUM)-current) { LENGTH_ERROR(errorPtr); } \
            unsigned int newCapacity; \
            if(current<(LOW)) { \
                unsigned int growth=3*(current+1)/5; \
                const unsigned int* choice=growth<extra?&extra:&growth; \
                newCapacity=current+*choice; \
            } else if(current<(HIGH)) { \
                unsigned int growth=(current+1)/2; \
                const unsigned int* choice=growth<extra?&extra:&growth; \
                newCapacity=current+*choice; \
            } else newCapacity=(MAXIMUM); \
            if(newCapacity>(MAXIMUM)) { LENGTH_ERROR(errorPtr); } \
            transaction.data=appMallocDummy__FUl(newCapacity*sizeof(VALUE)); \
            transaction.capacity=newCapacity; \
            transaction.remaining=(SIZE); \
            VALUE* out=(VALUE*)transaction.data+(SIZE)+transaction.size; \
            while(amount!=0) { \
                VALUE value={0}; \
                new(out) VALUE(value); \
                --amount; ++out; ++transaction.size; \
            } \
            /* Phase 3: retain original element order while copying backwards. */ \
            VALUE* source=(VALUE*)(DATA)+(SIZE); \
            VALUE* dest=(VALUE*)transaction.data+transaction.remaining; \
            while(source>(VALUE*)(DATA)) { \
                --dest; --source; \
                new(dest) VALUE(*source); \
                --transaction.remaining; ++transaction.size; \
            } \
            /* Phase 4: commit replacement; transaction now owns the old storage. */ \
            void* oldData=(DATA); \
            void* fresh=transaction.data; \
            unsigned int built=transaction.size; \
            (SIZE)=0; \
            unsigned int swappedCapacity=(CAPACITY); \
            transaction.data=oldData; \
            (CAPACITY)=transaction.capacity; transaction.capacity=swappedCapacity; \
            (DATA)=fresh; (SIZE)=built; transaction.size=0; \
        } \
    } else if(wanted<size) { (SIZE)=size-(size-wanted); } \
} while(0)

// Lazily resolve and register the shared Bp script controller.
inline void EnsurePauseScoreBpController() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_pauseBpCtrlRegistration);
            s_bpCtrlInitialized=1;
        }
        Sing_805DC5F0()=s_bpCtrl;
    }
}

#pragma use_lmw_stmw on
#pragma optimization_level 2
// Construct script objects, player-count-specific controls and zero score records.
// Requires playerCount in 1..4, type in 0..6, valid writable object storage.
// type 0/6 suppress scorecard/text creation; types 0/1/2/3/5 select score_btn_00.spd
// plus count-indexed tables; type 4 selects score_trg_btn_00.spd and fixed tables;
// type 6 skips layouts and sets interaction state 6. Gameplay mode names unresolved.
// Score script p2_score_1..4.b2d follows player count. Alternate group IDs apply to
// type 1 (also type 4 for one player); graph origins below are screen coordinates.
// The object owns layouts and both record vectors; post-hole argument is stored
// as an ABI word. Returns p; allocation/length errors may throw.
U8032D0FC* CWiiPauseScore_Ctor(U8032D0FC* p, int type, unsigned char playerCount, unsigned char holeCount, void* arg) {
    __alloca(0);
    CWiiPauseScore* score = AsCWiiPauseScore(p);
    register char* resourceData=pauseScorePartTotalGroupName;
    // Phase 1: base construction and empty member headers. The guard is armed
    // only after the vectors/base can safely be destroyed in reverse order.
    MPBase1_Ctor(p);
    p->vtable=__vt__14CWiiPauseScore; score->activationPending=0;
    score->scorecardSuppressed=0; score->blackSpriteEntryId=-1; score->blackSprite=0; score->scoreEntryId=-1; score->scoreObject=0;
    score->menuLayout=0; score->optionsLayout=0; score->alternatePlayerLabelLayout=0; score->playerLabelLayout=0; score->tieMarkerLayout=0; score->lossMarkerLayout=0;
    score->textTable=0; score->optionDialog=0; score->confirmationDialog=0; p->aD8=0; p->aDC=0; p->aE0=0; p->aE4=0; p->aE8=0; p->aEC=0;
    score->displayMode=type; p->playerCount=playerCount; p->holeCount=holeCount; p->arg5c=arg; score->interactionState=0;
    {
    ConstructionGuard construction(p);
    if(type==kScoreModeSuppressCard || type==kScoreModeSkipLayouts) score->scorecardSuppressed=1;
    EnsurePauseScoreBpController();
    CBpCtrl_CreateEntry(Sing_805DC5F0(),wiiPauseScoreStrings,&score->blackSpriteEntryId,0);
    EnsurePauseScoreBpController();
    score->blackSprite=IdMap_Lookup(Sing_805DC5F0(),score->blackSpriteEntryId);

    // Phase 2: select the score script and its player-group/graph-origin map.
    switch ((int)p->playerCount) {
        case 1: {
        EnsurePauseScoreBpController();
        CBpCtrl_CreateEntry(Sing_805DC5F0(),(char*)&wiiPauseScoreStrings+0x16,&score->scoreEntryId,0);
        p->ids[kCommonScoreGroup0]=0x4a; p->ids[kCommonScoreGroup1]=0x41; p->ids[kCommonScoreGroup2]=0x38; p->ids[kCommonScoreGroup3]=0x59; p->ids[kPlayer0Group]=0x50;
        if(score->displayMode==kScoreModeAlternateGroups || score->displayMode==kScoreModeFixedTables) p->ids[kPlayer0AlternateGroup]=0x5a; else p->ids[kPlayer0AlternateGroup]=0x4f;
        AsCWiiPauseScore(p)->graphOrigin0X=0x44; AsCWiiPauseScore(p)->graphOrigin0Y=0x105;
        break; } case 2: {
        EnsurePauseScoreBpController();
        CBpCtrl_CreateEntry(Sing_805DC5F0(),(char*)&wiiPauseScoreStrings+0x25,&score->scoreEntryId,0);
        p->ids[kCommonScoreGroup0]=0x56; p->ids[kCommonScoreGroup1]=0x4d; p->ids[kCommonScoreGroup2]=0x44; p->ids[kCommonScoreGroup3]=0x70; p->ids[kPlayer0Group]=0x5e; p->ids[kPlayer1Group]=0x5d;
        if(score->displayMode==kScoreModeAlternateGroups) { p->ids[kPlayer0AlternateGroup]=0x72; p->ids[kPlayer1AlternateGroup]=0x71; }
        else { p->ids[kPlayer0AlternateGroup]=0x5c; p->ids[kPlayer1AlternateGroup]=0x5b; }
        AsCWiiPauseScore(p)->graphOrigin0X=0x44; AsCWiiPauseScore(p)->graphOrigin0Y=0xf9; AsCWiiPauseScore(p)->graphOrigin1X=0x44; AsCWiiPauseScore(p)->graphOrigin1Y=0x133;
        break; } case 3: {
        EnsurePauseScoreBpController();
        CBpCtrl_CreateEntry(Sing_805DC5F0(),(char*)&wiiPauseScoreStrings+0x34,&score->scoreEntryId,0);
        p->ids[kCommonScoreGroup0]=0x5f; p->ids[kCommonScoreGroup1]=0x56; p->ids[kCommonScoreGroup2]=0x4d; p->ids[kCommonScoreGroup3]=0x84; p->ids[kPlayer0Group]=0x69; p->ids[kPlayer1Group]=0x68; p->ids[kPlayer2Group]=0x67;
        if(score->displayMode==kScoreModeAlternateGroups) { p->ids[kPlayer0AlternateGroup]=0x87; p->ids[kPlayer1AlternateGroup]=0x86; p->ids[kPlayer2AlternateGroup]=0x85; }
        else { p->ids[kPlayer0AlternateGroup]=0x66; p->ids[kPlayer1AlternateGroup]=0x65; p->ids[kPlayer2AlternateGroup]=0x64; }
        AsCWiiPauseScore(p)->graphOrigin0X=0x44; AsCWiiPauseScore(p)->graphOrigin0Y=0xf2; AsCWiiPauseScore(p)->graphOrigin1X=0x44; AsCWiiPauseScore(p)->graphOrigin1Y=0x11e; AsCWiiPauseScore(p)->graphOrigin2X=0x44; AsCWiiPauseScore(p)->graphOrigin2Y=0x149;
        break; } case 4: {
        EnsurePauseScoreBpController();
        CBpCtrl_CreateEntry(Sing_805DC5F0(),(char*)&wiiPauseScoreStrings+0x43,&score->scoreEntryId,0);
        p->ids[kCommonScoreGroup0]=0x6a; p->ids[kCommonScoreGroup1]=0x61; p->ids[kCommonScoreGroup2]=0x58; p->ids[kCommonScoreGroup3]=0x9a; p->ids[kPlayer0Group]=0x76; p->ids[kPlayer1Group]=0x75; p->ids[kPlayer2Group]=0x74; p->ids[kPlayer3Group]=0x73;
        if(score->displayMode==kScoreModeAlternateGroups) { p->ids[kPlayer0AlternateGroup]=0x9e; p->ids[kPlayer1AlternateGroup]=0x9d; p->ids[kPlayer2AlternateGroup]=0x9c; p->ids[kPlayer3AlternateGroup]=0x9b; }
        else { p->ids[kPlayer0AlternateGroup]=0x72; p->ids[kPlayer1AlternateGroup]=0x71; p->ids[kPlayer2AlternateGroup]=0x70; p->ids[kPlayer3AlternateGroup]=0x6f; }
        AsCWiiPauseScore(p)->graphOrigin0X=0x44; AsCWiiPauseScore(p)->graphOrigin0Y=0xe6; AsCWiiPauseScore(p)->graphOrigin1X=0x44; AsCWiiPauseScore(p)->graphOrigin1Y=0x10a; AsCWiiPauseScore(p)->graphOrigin2X=0x44; AsCWiiPauseScore(p)->graphOrigin2Y=0x12c; AsCWiiPauseScore(p)->graphOrigin3X=0x44; AsCWiiPauseScore(p)->graphOrigin3Y=0x14e;
        break; }
    }
    EnsurePauseScoreBpController();
    score->scoreObject=IdMap_Lookup(Sing_805DC5F0(),score->scoreEntryId);
    typedef unsigned int Table[5];
#define TABLE_AT(pointer) { ((unsigned int*)(pointer))[0], ((unsigned int*)(pointer))[1], ((unsigned int*)(pointer))[2], ((unsigned int*)(pointer))[3], ((unsigned int*)(pointer))[4] }
    // Each table has five words, indexed directly by player count (1..4).
    // layoutNamesByPlayerCount contains string ADDRESSES, despite the former
    // name "counts". The other tables pair group-list addresses with counts.
    Table layoutNamesByPlayerCount=TABLE_AT(((char*)(resourceData+0x26f8)));
    Table optionGroupsByPlayerCount=TABLE_AT(((char*)(resourceData+0x270c)));
    Table optionGroupCounts=TABLE_AT(((char*)(resourceData+0x2720)));
    Table playerLabelGroupsByPlayerCount=TABLE_AT(((char*)(resourceData+0x2734)));
    Table playerLabelGroupCounts=TABLE_AT(((char*)(resourceData+0x2748)));
    Table tieGroupsByPlayerCount=TABLE_AT(((char*)(resourceData+0x275c)));
    Table tieGroupCounts=TABLE_AT(((char*)(resourceData+0x2770)));
    Table lossGroupsByPlayerCount=TABLE_AT(((char*)(resourceData+0x2784)));
    Table lossGroupCounts=TABLE_AT(((char*)(resourceData+0x2798)));
#undef TABLE_AT
    // Phase 3: construct the mode-specific owned layouts after selecting their IDs.
    switch(score->displayMode) {
    case kScoreModeSuppressCard: case kScoreModeAlternateGroups: case 2: case 3: case 5:
        score->menuLayout=new LayoutResource(kScoreLayoutKind);
        Layout_LoadExtended(score->menuLayout,0,wiiPauseScoreStrings+0x52);
        score->optionsLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->optionsLayout,0,(void*)optionGroupsByPlayerCount[p->playerCount],optionGroupCounts[p->playerCount],(void*)layoutNamesByPlayerCount[p->playerCount]);
        if(p->playerCount==2) {
            score->alternatePlayerLabelLayout=new LayoutResource(kScoreLayoutKind);
            Layout_InitStandard(score->alternatePlayerLabelLayout,0,((char*)(resourceData+0xfa0)),1,wiiPauseScoreStrings+0x63);
        }
        score->playerLabelLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->playerLabelLayout,0,(void*)playerLabelGroupsByPlayerCount[p->playerCount],playerLabelGroupCounts[p->playerCount],(void*)layoutNamesByPlayerCount[p->playerCount]);
        score->tieMarkerLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->tieMarkerLayout,0,(void*)tieGroupsByPlayerCount[p->playerCount],tieGroupCounts[p->playerCount],(void*)layoutNamesByPlayerCount[p->playerCount]);
        score->lossMarkerLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->lossMarkerLayout,0,(void*)lossGroupsByPlayerCount[p->playerCount],lossGroupCounts[p->playerCount],(void*)layoutNamesByPlayerCount[p->playerCount]);
        break;
    case kScoreModeFixedTables:
        score->menuLayout=new LayoutResource(kScoreLayoutKind);
        Layout_LoadExtended(score->menuLayout,0,wiiPauseScoreStrings+0x72);
        score->optionsLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->optionsLayout,0,((char*)(resourceData+0x6c8)),20,wiiPauseScoreStrings+0x87);
        score->playerLabelLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->playerLabelLayout,0,((char*)(resourceData+0xfe0)),2,wiiPauseScoreStrings+0x87);
        score->tieMarkerLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->tieMarkerLayout,0,((char*)(resourceData+0x12a0)),9,wiiPauseScoreStrings+0x87);
        score->lossMarkerLayout=new LayoutResource(kScoreLayoutKind);
        Layout_InitStandard(score->lossMarkerLayout,0,((char*)(resourceData+0x1c98)),9,wiiPauseScoreStrings+0x87);
        break;
    case kScoreModeSkipLayouts: score->interactionState=6; break;
    }
    // Phase 4: size score payloads by holes and player-holes, zero all words,
    // enable ten menu controls, then obtain the optional numeric text renderer.
    // RESIZE_VALUES(type, data, size, capacity, requested records,
    //               maximum records, 60%-growth cutoff, 50%-growth cutoff).
    // aD8/aDC/aE0 = holeInfo; aE4/aE8/aEC = playerHoleScores. These
    // unsigned ABI words must stay in the original placement/EH scope.
    RESIZE_VALUES(PairValue,
        p->aD8, p->aDC, p->aE0, // owning holeInfo vector: data/count/capacity
        p->holeCount, kMaxHoleRecords, kHoleRecordSmallGrowthLimit, kHoleRecordHalfGrowthLimit);
    RESIZE_VALUES(TripleValue,
        p->aE4, p->aE8, p->aEC, // owning playerHoleScores vector: data/count/capacity
        p->holeCount*p->playerCount, kMaxPlayerHoleRecords, kPlayerHoleSmallGrowthLimit, kPlayerHoleHalfGrowthLimit);
    U8032D0FC* enabled=p;
    for(int i=0;i<kMenuControlCount;i+=5,enabled=(U8032D0FC*)((char*)enabled+20)) {
        enabled->enabled[0]=1; enabled->enabled[1]=1; enabled->enabled[2]=1; enabled->enabled[3]=1; enabled->enabled[4]=1;
    }
    if(score->scorecardSuppressed==0) {
        if(!Sing_805DC830()) {
            if(!s_pointerVectorInitialized) { PointerVector_InitReserved(s_fontNumDrawRegistry); __register_global_object((void*)s_fontNumDrawRegistry,(void*)PointerVector_DestroyStorage,(void*)s_pauseFontRegistryRegistration); s_pointerVectorInitialized=1; }
            Sing_805DC830()=s_fontNumDrawRegistry;
        }
        score->textTable=CFontNumDraw_CreateInVector(Sing_805DC830());
    }
    construction.object=0;
    }
    return p;
}

} // extern "C"
