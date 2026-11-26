// Computer player data: the four random-adjustment tables and the stroke score table, which
// are loaded from shared files, the character stat bars, and the computer score generator.

#include <decomp/static_init.h>

// The five IFF stat columns use this order in character, part and caddie records.
enum GolfStat {
    kPower = 0,
    kControl = 1,
    kAccuracy = 2,
    kSpin = 3,
    kCurve = 4,
    kStatCount = 5
};
enum {
    kCharacterPartSlots = 24,
    kHolesPerRound = 18,
    kComputerScoreVariants = 8,
    kComputerHoleRowBytes = 0x28,
    kRandomPercentRange = 100,
    kRandomSignMidpoint = 0x3FFF,
    kStatBarMaximum = 30
};
// Caller evidence: CComShotProc/ComShot randomize a power ratio and an impact
// value centered at 100; putting randomizes candidate z (power) and x (radians).
// The former "approach" pair is selected when the target index is -1, so its
// physical shot kind is not asserted. Both no-target categories scale by distance.
enum ComputerAdjustmentCategory {
    kNormalPowerAdjustment = 0,
    kNormalImpactAdjustment = 1,
    kPuttPowerAdjustment = 2,
    kPuttAngleAdjustment = 3,
    kNoTargetPowerAdjustment = 4,
    kNoTargetImpactAdjustment = 5,
    kSpecialPowerAdjustment = 6,
    kSpecialImpactAdjustment = 7
};
// Table rows start with a 16-bit row key; remaining shorts are cumulative cutoffs or adjustments.
// Cutoff rows are row index+2. Adjustment tables are rows 0 (power) and 1
// (impact/angle). A selected bucket 0..5 reads shorts 1..6 after the row key.
// If no cutoff matches, bucket=-1 deliberately reads short 0, the row key.
// The asset must supply a usable fallback there; no clamp is performed.
struct ComputerRandomRow {
    short rowKey;
    short values[6];
};
// The six score bonuses are indexed by the outcome, rather than by the hole number.
struct ComputerCourseScoreBonuses {
    int holeInOne, albatross, eagle, birdie, par, other;
};
struct ComputerHoleRecord {
    char beforePar[0x18];
    unsigned char par;
    char tail[7];
};
struct ComputerPlayerPosition {
    char beforePosition[0x1C];
    float ballPosition[3];
};
struct EnemyStatBytes {
    char beforeEquipmentBonuses[0x60];
    signed char equipmentBonuses[kStatCount];
    char beforeFallbackStats[0x36];
    signed char fallbackStats[kStatCount];
};
typedef char ComputerHoleRecord_size[sizeof(ComputerHoleRecord) == 0x20 ? 1 : -1];
typedef char EnemyFallbackOffset[((int)&((EnemyStatBytes*)0)->fallbackStats == 0x9B) ? 1 : -1];

#include "classes/CCharaEntry.h"

#include "iff/IffStruct.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CEnemyRenderRecord.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/vector.h"
#include "game/wvector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CComStrokeScore.h"
#include "classes/CComRandomUse.h"
#include "classes/CComMatchTbl.h"
#include "classes/CComStrokeUse.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup {
    char* base;
    bool active;
    // Arm cleanup for a partially constructed render-context aggregate.
    explicit RenderContextConstructionCleanup(void* object) : base((char*)object), active(true) {}
    // Destroy avatar records and retained vector allocations if initialization throws.
    ~RenderContextConstructionCleanup() {
        if (active) {
            CharacterPreviewTable_dtor(base + 0x20D0, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + 0x20A0 + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Initialize render-context state with cleanup active if construction throws.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

extern const float kFloat_6_2831855;
extern float s_computerRandomInterfaceTwoPi;
extern float s_computerRandomTableTwoPi;
extern float s_computerStrokeInterfaceTwoPi;
extern unsigned char __vt__13CComRandomUse[];
extern unsigned char __vt__13CComStrokeUse[];
extern unsigned char __vt__15CComStrokeScore[];
extern unsigned char computerCourseScoreBonuses[];
extern void* g_sharedDoc;
extern const float kFloat_0_805E0300;                  // Original scalar value: 0.
extern const float kFloat_30_805E0304;                 // Original scalar value: 30.
extern const float kFloat_0_805E02E0;                  // Original scalar value: 0.
extern const double kDouble_4503601774854144_805E02D8; // Original scalar value: 4503601774854144.
extern const double kDouble_0_01_805E02E8;             // Original scalar value: 0.01.
extern const float kFloat_1_805E02F0;                  // Original scalar value: 1.
extern const float kFloat_0_0174532924_805E02F4;       // Original scalar value: 0.0174532924.
extern const float kFloat_10_805E02F8;                 // Original scalar value: 10.
extern const double kDouble_4503601774854144_805E02D0; // Original scalar value: 4503601774854144.
extern const float kFloat_1_39999998_805E02C0;         // Original scalar value: 1.39999998.
extern const float kFloat_70_805E02C4;                 // Original scalar value: 70.
extern const float kFloat_1_66600001_805E02C8;         // Original scalar value: 1.66600001.
extern signed char s_renderContextInitialized;
extern void* g_sharedDoc;
extern signed char s_bpCtrlInitialized;
extern const unsigned int kCharacterStatBarInitialXBits;
extern const unsigned int kCharacterStatBarUnitYBits;
extern unsigned char s_renderContext[];
extern unsigned char s_bpCtrl[];
extern unsigned char s_renderContextDtorRegistration_80506F1C[];
extern unsigned char s_bpCtrlDtorRegistration_80506F04[];
extern unsigned char __vt__13CComRandomTbl[];
extern const unsigned char computerShotDiagnosticFormats[];

void* CSharedFilePtr_GetLoadedBuffer(void*);
void* SharedFile_Create(void*, int);
int Table_GetEntry(void*, int);
int SharedDoc_GetCourseId();
int GolfPlayer_GetEntryByIndex(void*, int);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
float Random_RangeFloat(int, float, float);
void __dl__FPv(void*);
void* memset(void*, int, unsigned int);
void SaveData_InitPlayerRecord(void*);

void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void __dt__13RenderContextFv(void*);
void RenderContext_ResetPlayerRecords(void*);
int ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_FindPartData(void*, int);
void* ItemInventory_GetCharacterIdAt(void*, int);
void* ItemInventory_FindCharacterData(void*, void*);
void* ItemInventory_GetCaddieIdAt(void*, int);
void* ItemInventory_FindCaddieData(void*, void*);
void* CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, int);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetScale(void*, float*);

// Construction macro contract: home is the static render-context base; saved
// is read only on first construction. It initializes four save records, four
// pointer vectors and four avatar records, registers cleanup, then installs the
// singleton. Replacing this expansion with an inline call changes compiled code.
#define GET_PLAYERS(saved)                                                                                            \
    if (!Sing_805DC648()) {                                                                                           \
        if (!(signed char)s_renderContextInitialized) {                                                               \
            unsigned char* first = saved;                                                                             \
            unsigned char* end = home + 0x20a0;                                                                       \
            do {                                                                                                      \
                SaveData_InitPlayerRecord(first);                                                                     \
                first += 0x828;                                                                                       \
            } while (first < end);                                                                                    \
            ((IntVectorArray*)(end))->__ct();                                                                         \
            unsigned char* members = home + 0x20d0;                                                                   \
            __construct_array(members, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xa4, 4); \
            AsCEnemyRenderTable(members)->count = 0;                                                                  \
            InitializeNewRenderContext(saved);                                                                        \
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,                                 \
                                     s_renderContextDtorRegistration_80506F1C);                                       \
            s_renderContextInitialized = 1;                                                                           \
        }                                                                                                             \
        Sing_805DC648() = home;                                                                                       \
    }
// Lazily initialize the BP controller used by the stat bars.
static inline void EnsureStatBarControl() {
    if (!((int&)Sing_805DC5F0())) {
        if (!(signed char)s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_80506F04);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}
// Add one character IFF column to accum[n] after resolving render record ->
// character inventory ID -> IFF record. Caller owns accum; all lookups are borrowed.
#define ADD_BASE(n, statIndex, saved)                                                                             \
    GET_PLAYERS(saved);                                                                                           \
    {                                                                                                             \
        void* manager = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);                                       \
        void* p = ItemInventory_GetCharacterIdAt(                                                                 \
            manager, ((CEnemyRenderRecord*)((char*)Sing_805DC648() + offset + 0x20d0))->characterInventoryIndex); \
        p = ItemInventory_FindCharacterData(manager, p);                                                          \
        accum[n] += (int)((IFF_STRUCT::sChar*)p)->stats[statIndex];                                               \
    }
// Set one stat-bar scale through BP lookup. value is the caller's two-float
// scale pair; id is evaluated after EnsureStatBarControl, as in the original.
#define UPDATE_BAR(id)      \
    EnsureStatBarControl(); \
    UIGroup_SetScale(       \
        UIGroup_GetOrCreateById(IdMap_Lookup((void*)((int&)Sing_805DC5F0()), (int)characterStatsScriptId), id), value)

// 0x8024F60C: keep singleton setup in the loop's bounds check.
static inline int EnemyRenderCount(unsigned char* home, unsigned char* volatile& saved) {
    if (!Sing_805DC648()) {
        if (!(signed char)s_renderContextInitialized) {
            unsigned char* end;
            unsigned char* first = saved;
            end = home + 0x20a0;
            do {
                SaveData_InitPlayerRecord(first);
                first += 0x828;
            } while (first < end);
            ((IntVectorArray*)(end))->__ct();
            unsigned char* members = home + 0x20d0;
            __construct_array(members, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xa4, 4);
            AsCEnemyRenderTable(members)->count = 0;
            {
                RenderContextConstructionCleanup cleanup(saved);
                RenderContext_ResetPlayerRecords(saved);
                cleanup.active = false;
            }
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv,
                                     s_renderContextDtorRegistration_80506F1C);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = home;
    }
    return AsCEnemyRenderTable((char*)Sing_805DC648() + 0x20d0)->count;
}

// Sum character, clothing, equipment and caddie stat bonuses and update power/control/spin bars.
// Stat source map: human records sum 24 clothing parts + five signed-byte
// equipment bonuses + character base + optional caddie; CPU records use the five
// signed-byte fallback stats directly. Inventory indices first resolve to IDs,
// then to IFF records. All records/handles are borrowed for this update.
// UI IDs per player: power=0x400+5*p, control=0x3FF+5*p, spin=0x3FE+5*p.
// Power width=1.4*stat; control/spin width=70 at >=30, otherwise 1.666*1.4*stat.
// value[1] stays the initialized scale component; only value[0] changes per bar.
void CCharaEntry::UpdateStatBars() {
    float value[2];
    *(unsigned int*)&value[0] = kCharacterStatBarInitialXBits;
    *(unsigned int*)&value[1] = kCharacterStatBarUnitYBits;
    int accum[kStatCount] = {};
    int loop = 0;
    int powerBarId = 0x400;
    int controlBarId = 0x3ff;
    int spinBarId = 0x3fe;
    int offset = 0;
    unsigned char* home = s_renderContext;
    unsigned char* volatile savedEquip = home;
    unsigned char* volatile savedChars = home;
    unsigned char* volatile savedItems = home;
    unsigned char* volatile savedBase0 = home;
    unsigned char* volatile savedBase1 = home;
    unsigned char* volatile savedBase2 = home;
    unsigned char* volatile savedBase3 = home;
    unsigned char* volatile savedBase4 = home;
    unsigned char* volatile savedTest = home;
    unsigned char* volatile savedFlags = home;
    float scale = kFloat_1_39999998_805E02C0;
    float scale2 = kFloat_1_66600001_805E02C8;
    // 0x8024F60C unit: refresh the render table before every bounds test.
    while (loop < EnemyRenderCount(home, savedTest)) {
        for (int z = 0; z < kStatCount; ++z)
            accum[z] = 0;
        // Phase 1: pick the stat source for this render record. Singleton reads stay
        // at each original access so construction and exception behavior are preserved.
        GET_PLAYERS(savedFlags);
        if (!((((CEnemyRenderRecord*)((char*)Sing_805DC648() + offset + 0x20d0))->flags >> 15) &
              1)) { // original +0x20EE
            int step = 0;
            for (int i = 0; i < kCharacterPartSlots; ++i, step += 2) {
                GET_PLAYERS(savedItems);
                if (ItemInventory_GetPartIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory),
                                              ((CEnemyRenderRecord*)((char*)Sing_805DC648() + (offset + step) + 0x20d0))
                                                  ->partInventoryIndices[0])) {
                    int entryOffset = offset + step;
                    for (int j = 0; j < kStatCount; ++j) {
                        GET_PLAYERS(home);
                        void* manager = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
                        int p = ItemInventory_GetPartIdAt(
                            manager, ((CEnemyRenderRecord*)((char*)Sing_805DC648() + entryOffset + 0x20d0))
                                         ->partInventoryIndices[0]);
                        p = ItemInventory_FindPartData(manager, p);
                        int amount = (short)((IFF_STRUCT::sPart*)p)->stats[j];
                        accum[j] = accum[j] + amount;
                    }
                }
            }
            for (int j = 0; j < kStatCount; ++j) {
                GET_PLAYERS(savedChars);
                accum[j] += ((EnemyStatBytes*)((char*)Sing_805DC648() + (j + offset) + 0x20D0))->equipmentBonuses[0];
            }
            // Phase 2: add character base columns, then the equipped caddie columns.
            ADD_BASE(kPower, kPower, savedBase0);
            ADD_BASE(kControl, kControl, savedBase1);
            ADD_BASE(kAccuracy, kAccuracy, savedBase2);
            ADD_BASE(kSpin, kSpin, savedBase3);
            ADD_BASE(kCurve, kCurve, savedBase4);
            for (int j = 0; j < kStatCount; ++j) {
                GET_PLAYERS(savedEquip);
                void* manager = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
                void* p = ItemInventory_GetCaddieIdAt(manager,
                                                      ((CEnemyRenderRecord*)(offset + (char*)Sing_805DC648() + 0x20d0))
                                                          ->caddieInventoryIndex); // original +0x20DC
                p = ItemInventory_FindCaddieData(manager, p);
                if (p)
                    accum[j] += (short)((IFF_STRUCT::sCaddie*)p)->stats[j];
            }
        } else {
            for (int j = 0; j < kStatCount; ++j) {
                GET_PLAYERS(home);
                accum[j] += ((EnemyStatBytes*)((char*)Sing_805DC648() + (j + offset) + 0x20D0))->fallbackStats[0];
            }
        }
        // Phase 3: convert aggregate stats to layout widths; advance all three IDs.
        value[0] = scale * accum[kPower];
        UPDATE_BAR(powerBarId);
        value[0] = accum[kControl] >= kStatBarMaximum ? kFloat_70_805E02C4 : scale2 * (scale * accum[kControl]);
        UPDATE_BAR(controlBarId);
        value[0] = accum[kSpin] >= kStatBarMaximum ? kFloat_70_805E02C4 : scale2 * (scale * accum[kSpin]);
        UPDATE_BAR(spinBarId);
        offset += 0xa4;
        powerBarId += 5;
        controlBarId += 5;
        spinBarId += 5;
        ++loop;
    } // original +0x2360
}
#undef UPDATE_BAR
#undef ADD_BASE

#undef GET_PLAYERS

struct CComRandomUseView {
    void* vtable;
    int shotTableHandle;
    int puttTableHandle;
    int noTargetTableHandle;
    int specialTableHandle;
};

struct Fn80250A80Object {
    char precedingStorage[0x2E4];
    void* strokeScoreFile;
};

struct Fn80250A44Object {
    char precedingStorage[0x2E4];
    void* strokeScoreFile;
};

struct Fn802503F4Object {
    char precedingStorage[4];
    void* shotTableHandle;
};

struct Fn80250430Object {
    char precedingStorage[8];
    void* puttTableHandle;
    void* noTargetTableHandle;
    void* specialTableHandle;
};

struct CComStrokeUseView {
    void* vtable;
    char baseStorage[0x2E0];
    int strokeScoreFile;
};

struct Fn802504E4Object {
    void* vtable;
    void* tableHandles[4];
};

struct Fn8025036CObject {
    void* vtable;
    void* fields[4];
};
struct Fn802509C8Object {
    void* vtable;
    char baseStorage[0x2E0];
    void* strokeScoreFile;
};
struct Fn80250AB0Object {
    char precedingStorage[0x12];
    unsigned char data[kHolesPerRound][kComputerHoleRowBytes];
};
struct ComputerStrokeInput {
    short first;
    unsigned char b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13;
};

// Ignore diagnostic formatting calls retained by the original computer-shot code.
void ComputerShot_DiscardDiagnostic(...) {}

#pragma push
#pragma use_lmw_stmw off
// Choose a weighted adjustment from the category table, then randomly add or subtract it from the requested value.
// index selects a character/difficulty cutoff row; category must be 0..7.
// Returns requestedValue +/- delta without clamping; a zero delta consumes no
// sign draw. Normal/special/power deltas are requestedValue*tableValue/100.
// No-target deltas multiply by (0.01 + 1/horizontalDistance); distance uses the
// hole vector at +0x0C minus ball position, Y cleared, in game coordinate units.
// Putting angle table values are tenths of a degree, converted to radians.
// No guard exists for zero distance or an unsupported category.
float CComRandomUse::AdjustValue(int index, int category, float requestedValue) {
    Fn802504E4Object* self = (Fn802504E4Object*)this;
    int tableIndex = 0;
    switch (category) {
    case kNormalPowerAdjustment:
    case kNormalImpactAdjustment:
        tableIndex = 0;
        break;
    case kPuttPowerAdjustment:
    case kPuttAngleAdjustment:
        tableIndex = 1;
        break;
    case kNoTargetPowerAdjustment:
    case kNoTargetImpactAdjustment:
        tableIndex = 2;
        break;
    case kSpecialPowerAdjustment:
    case kSpecialImpactAdjustment:
        tableIndex = 3;
        break;
    }
    void* archive = CSharedFilePtr_GetLoadedBuffer(self->tableHandles[tableIndex]);
    struct ArchiveLocal {
        void* data;
        void* vtable;
    };
    volatile ArchiveLocal result;
    result.data = 0;
    if (archive != 0)
        result.data = archive;
    void* entry = (void*)((int (*)(void*, int, void*))Table_GetEntry)(result.data, index + 2,
                                                                      result.vtable = __vt__13CComRandomTbl);
    short* adjustmentRow;
    int randomValue = rand();
    int randomPercent = randomValue % kRandomPercentRange;
    ComputerShot_DiscardDiagnostic(computerShotDiagnosticFormats, index);
    ComputerShot_DiscardDiagnostic(computerShotDiagnosticFormats + 0x11, randomPercent);
    int threshold = (short)randomPercent;
    // Phase 1: draw 0..99 and choose the first nonnegative cumulative cutoff >= draw.
    int adjustmentIndex = -1;
    if (((ComputerRandomRow*)entry)->values[0] >= 0 && ((ComputerRandomRow*)entry)->values[0] >= threshold)
        adjustmentIndex = 0;
    else if (((ComputerRandomRow*)entry)->values[1] >= 0 && ((ComputerRandomRow*)entry)->values[1] >= threshold)
        adjustmentIndex = 1;
    else if (((ComputerRandomRow*)entry)->values[2] >= 0 && ((ComputerRandomRow*)entry)->values[2] >= threshold)
        adjustmentIndex = 2;
    else if (((ComputerRandomRow*)entry)->values[3] >= 0 && ((ComputerRandomRow*)entry)->values[3] >= threshold)
        adjustmentIndex = 3;
    else if (((ComputerRandomRow*)entry)->values[4] >= 0 && ((ComputerRandomRow*)entry)->values[4] >= threshold)
        adjustmentIndex = 4;
    else if (((ComputerRandomRow*)entry)->values[5] >= 0 && ((ComputerRandomRow*)entry)->values[5] >= threshold)
        adjustmentIndex = 5;
    // Phase 2: select the magnitude row; bucket=-1 reads its leading fallback short.
    int adjustmentTable = 0;
    switch (category) {
    case kNormalPowerAdjustment:
    case kPuttPowerAdjustment:
    case kNoTargetPowerAdjustment:
    case kSpecialPowerAdjustment:
        adjustmentTable = Table_GetEntry(result.data, 0);
        break;
    case kNormalImpactAdjustment:
    case kPuttAngleAdjustment:
    case kNoTargetImpactAdjustment:
    case kSpecialImpactAdjustment:
        adjustmentTable = Table_GetEntry(result.data, 1);
        break;
    }
    ComputerShot_DiscardDiagnostic(computerShotDiagnosticFormats + 0x23, adjustmentIndex);
    adjustmentRow = (short*)((char*)adjustmentTable + adjustmentIndex * 2);
    ComputerShot_DiscardDiagnostic(computerShotDiagnosticFormats + 0x34, adjustmentRow[1]);
    int value = adjustmentRow[1];
    // Phase 3: convert magnitude to input units, then make one random sign draw.
    if (value == 0)
        return requestedValue;
    float adjustment = kFloat_0_805E02E0;
    switch (category) {
    case kNormalPowerAdjustment:
    case kNormalImpactAdjustment:
    case kSpecialPowerAdjustment:
    case kSpecialImpactAdjustment:
        adjustment = (float)(kDouble_0_01_805E02E8 * (double)(requestedValue * (float)value));
        break;
    case kNoTargetPowerAdjustment:
    case kNoTargetImpactAdjustment: {
        void* g = g_sharedDoc;
        void* golfDocument = *(void**)&AsCSharedDoc(g)->golfDoc; // original +0xB90
        int playerIndex = AsCGolfDoc(golfDocument)->currentPlayer;
        int playerEntry = GolfPlayer_GetEntryByIndex(golfDocument, playerIndex);
        void* g2 = g_sharedDoc;
        void* obj2 = *(void**)&AsCSharedDoc(g2)->golfDoc;
        unsigned char holeIndex = AsCGolfDoc(obj2)->currentHoleIndex;
        void* base = AsCGolfDoc(obj2)->holeRecords;
        char* holeRecord = (char*)base + (holeIndex << 5);
        struct Vec {
            int x, y, z;
        };
        Vec distanceVector;
        Vec holePosition = *(Vec*)(holeRecord + 0xc);
        __mi__FRC7WVectorRC7WVector(&distanceVector, &holePosition,
                                    ((ComputerPlayerPosition*)playerEntry)->ballPosition);
        *(float*)((char*)&distanceVector + 4) = kFloat_0_805E02E0;
        float distance = (*(WVector*)&distanceVector).Magnitude();
        adjustment = (float)((kDouble_0_01_805E02E8 + (double)(kFloat_1_805E02F0 / distance)) *
                             (double)(requestedValue * (float)adjustmentRow[1]));
        break;
    }
    case kPuttPowerAdjustment:
        adjustment = (float)(kDouble_0_01_805E02E8 * (double)(requestedValue * (float)value));
        break;
    case kPuttAngleAdjustment:
        adjustment = kFloat_0_0174532924_805E02F4 * ((float)value / kFloat_10_805E02F8);
        break;
    }
    if (rand() < kRandomSignMidpoint)
        return requestedValue + adjustment;
    return requestedValue - adjustment;
}
#pragma pop

// Initialize computer-score storage and clear its per-hole score payload.
CComStrokeUse::CComStrokeUse() {
    CComStrokeUseView* self = (CComStrokeUseView*)this;
    self->vtable = __vt__13CComStrokeUse;
    self->strokeScoreFile = 0;
    memset(((Fn80250AB0Object*)self)->data, 0, sizeof(((Fn80250AB0Object*)self)->data));
}

// Report ready after all four computer-adjustment table files are loaded.
int CComRandomUse::AreTablesLoaded() {
    Fn802504E4Object* self = (Fn802504E4Object*)this;
    for (int i = 0; i < 4; i++) {
        if (CSharedFilePtr_GetLoadedBuffer(self->tableHandles[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

// Initialize the four computer-adjustment file handles.
CComRandomUse::CComRandomUse() {
    CComRandomUseView* self = (CComRandomUseView*)this;
    self->shotTableHandle = 0;
    self->vtable = &__vt__13CComRandomUse;
    self->puttTableHandle = 0;
    self->noTargetTableHandle = 0;
    self->specialTableHandle = 0;
}

// Test whether the computer-score table has a loaded buffer.
int CComStrokeUse::IsFileLoaded() {
    Fn80250A80Object* self = (Fn80250A80Object*)this;
    return CSharedFilePtr_GetLoadedBuffer(self->strokeScoreFile) != 0;
}

// Request the computer-score table resource.
int CComStrokeUse::RequestFileLoad(void* arg) {
    Fn80250A44Object* self = (Fn80250A44Object*)this;
    self->strokeScoreFile = SharedFile_Create(arg, 0);
    return 1;
}

// The table buffer is owned by its shared file, so destruction releases nothing.
CComMatchTbl::~CComMatchTbl() {}

// Delete the empty computer-score interface wrapper when requested.
void* dtor_8025092C(void* self, int flags) {
    if (self != 0 && flags > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Request the regular-shot adjustment table resource.
int CComRandomUse::RequestShotTableLoad(void* arg) {
    Fn802503F4Object* self = (Fn802503F4Object*)this;
    self->shotTableHandle = SharedFile_Create(arg, 0);
    return 1;
}

// Request the putting adjustment table resource.
int CComRandomUse::RequestPuttTableLoad(void* arg) {
    Fn80250430Object* self = (Fn80250430Object*)this;
    self->puttTableHandle = SharedFile_Create(arg, 0);
    return 1;
}

// Request the no-target adjustment table resource (asset name: "approach").
int CComRandomUse::RequestNoTargetTableLoad(void* arg) {
    Fn80250430Object* self = (Fn80250430Object*)this;
    self->noTargetTableHandle = SharedFile_Create(arg, 0);
    return 1;
}

// Request the special-shot adjustment table resource.
int CComRandomUse::RequestSpecialTableLoad(void* arg) {
    Fn80250430Object* self = (Fn80250430Object*)this;
    self->specialTableHandle = SharedFile_Create(arg, 0);
    return 1;
}

// Initialize the computer-random interface angle constant.
void ComputerRandomInterface_InitAngleConstant() {
    s_computerRandomInterfaceTwoPi = kFloat_6_2831855;
}

// Initialize the computer-random table angle constant.
void ComputerRandomTable_InitAngleConstant() {
    s_computerRandomTableTwoPi = kFloat_6_2831855;
}

// Initialize the computer-score interface angle constant.
void ComputerStrokeInterface_InitAngleConstant() {
    s_computerStrokeInterfaceTwoPi = kFloat_6_2831855;
}

#pragma push
#pragma use_lmw_stmw on
// Release all four adjustment-table handles before optional deletion.
CComRandomUse::~CComRandomUse() {
    Fn8025036CObject* self = (Fn8025036CObject*)this;
    self->vtable = __vt__13CComRandomUse;
    for (int i = 0; i < 4; i++) {
        if (self->fields[i] != 0) {
            CSharedFileHandle::Destroy(self->fields[i], 1);
            self->fields[i] = 0;
        }
    }
}
#pragma pop

// Release the computer-score table handle before optional deletion.
CComStrokeUse::~CComStrokeUse() {
    Fn802509C8Object* self = (Fn802509C8Object*)this;
    self->vtable = __vt__13CComStrokeUse;
    if (self->strokeScoreFile != 0) {
        CSharedFileHandle::Destroy(self->strokeScoreFile, 1);
        self->strokeScoreFile = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on

struct StrokeHeader {
    short first;
    unsigned char b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13;
    // Copy the packed score-generation header without interpreting opaque option bytes.
    StrokeHeader(short first_, const ComputerStrokeInput& i)
        : first(first_), b2(i.b2), b3(i.b3), b4(i.b4), b5(i.b5), b6(i.b6), b7(i.b7), b8(i.b8), b9(i.b9), b10(i.b10),
          b11(i.b11), b12(i.b12), b13(i.b13) {}
};
// Draw an integer from [low, high); equal bounds return that bound without consuming randomness.
inline short StrokeRandom(short low, short high) {
    if (low == high)
        return low;
    return low + rand() % (high - low);
}
#pragma opt_loop_invariants off
// Generate up to eight 18-hole computer score variants from stroke bounds, hole par and course bonuses.
void Generate_ComputerStrokeScores(Fn80250AB0Object* self, ComputerStrokeInput* input) {
    struct StrokeRow {
        unsigned char bytes[0x28];
    };
    short strokeCount;
    int offset;
    StrokeRow* outputRow;
    short* upperStrokeBounds;
    short* lowerStrokeBounds;
    short* baseStrokeCounts;
    int categoryRowIndex;
    // Phase 1: copy the input header, resolve table/course and select course bonuses.
    // input byte +3 begins eight signed variant category indices; output is hole-major:
    // each 0x28-byte hole row contains eight 4-byte entries (stroke byte, pad, score short).
    signed char* category = (signed char*)input + 3;
    *(StrokeHeader*)((char*)self + 4) = StrokeHeader(*(volatile short*)input, *input);
    void* archive = CSharedFilePtr_GetLoadedBuffer(((Fn80250A80Object*)self)->strokeScoreFile);
    CComStrokeScore arch;
    arch.data = 0;
    if (archive)
        arch.data = archive;
    arch.vtable = __vt__15CComStrokeScore;
    short* initialRow = (short*)(Table_GetEntry(arch.data, 0) + 2);
    int courseIndex = SharedDoc_GetCourseId();
    void* sharedDocument = g_sharedDoc;
    void* golfDocument = *(void**)&AsCSharedDoc(sharedDocument)->golfDoc;
    if (AsCGolfDoc(golfDocument)->tutorialStage < 6)
        courseIndex = 0;
    unsigned char* courseScoreBonuses = computerCourseScoreBonuses + (courseIndex & 0xff) * 0x18;
    int variantIndex;
    int holeIndex;
    unsigned char* scoreEntry;
    int lowerBoundsRow;
    int upperBoundsRow;
    variantIndex = 0;
    offset = 0;
    // Phase 2: generate one variant across up to 18 holes; RNG order is significant.
    do {
        signed char index = *category;
        categoryRowIndex = index * 2 + 1;
        lowerBoundsRow = Table_GetEntry(arch.data, categoryRowIndex + 1);
        upperBoundsRow = Table_GetEntry(arch.data, categoryRowIndex);
        baseStrokeCounts = initialRow;
        outputRow = (StrokeRow*)self->data;
        upperStrokeBounds = (short*)(upperBoundsRow + 2);
        lowerStrokeBounds = (short*)(lowerBoundsRow + 2);
        holeIndex = 0;
        do {
            scoreEntry = (unsigned char*)outputRow + offset;
            strokeCount = *lowerStrokeBounds;
            short exclusiveUpperBound = *upperStrokeBounds + 1;
            strokeCount = StrokeRandom(strokeCount, exclusiveUpperBound);
            int sum = strokeCount + *baseStrokeCounts;
            scoreEntry[0] = (unsigned char)sum;
            int signedStrokeCount = (signed char)scoreEntry[0];
            void* holeSharedDocument = g_sharedDoc;
            void* holeGolfDocument = *(void**)&AsCSharedDoc(holeSharedDocument)->golfDoc;
            void* holeRecords = AsCGolfDoc(holeGolfDocument)->holeRecords;
            int holePar = ((ComputerHoleRecord*)holeRecords)[(unsigned char)holeIndex].par;
            int strokesOverPar = signedStrokeCount - holePar;
            float randomScore = Random_RangeFloat(-1, kFloat_0_805E0300, kFloat_30_805E0304);
            int initialScore = (signed char)(int)randomScore;
            *(short*)(scoreEntry + 2) = (short)initialScore;
            // Preserve the original early stop: a hole-in-one ends this variant's hole loop.
            if ((int)scoreEntry[0] == 1) {
                int holeInOneBonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->holeInOne;
                *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + holeInOneBonus);
                break;
            } else
                switch (strokesOverPar) {
                case 0: {
                    int bonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->par;
                    *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + bonus);
                    break;
                }
                case -1: {
                    int bonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->birdie;
                    *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + bonus);
                    break;
                }
                case -2: {
                    int bonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->eagle;
                    *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + bonus);
                    break;
                }
                case -3: {
                    int bonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->albatross;
                    *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + bonus);
                    break;
                }
                default: {
                    int bonus = ((ComputerCourseScoreBonuses*)courseScoreBonuses)->other;
                    *(short*)(scoreEntry + 2) = (short)(*(unsigned short*)(scoreEntry + 2) + bonus);
                    break;
                }
                }
            ++holeIndex;
            ++outputRow;
            ++upperStrokeBounds;
            ++lowerStrokeBounds;
            ++baseStrokeCounts;
        } while (holeIndex < kHolesPerRound);
        ++variantIndex;
        offset += 4;
        ++category;
    } while (variantIndex < kComputerScoreVariants);
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622D4", _ctors_u8024F60C_0, ComputerRandomInterface_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU804622D4", _ctors_u8024F60C_1, ComputerRandomTable_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU804622D4", _ctors_u8024F60C_2, ComputerStrokeInterface_InitAngleConstant);
#pragma pop
