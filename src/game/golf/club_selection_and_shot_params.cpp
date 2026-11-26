// Golf club selection and shot parameters: the club list and its name registry, club choice for
// the player and the CPU, the shot's power, launch and impact values, and the golf clock actor.

#include <decomp/static_init.h>
// Field offsets are checked in the class headers; original function addresses stay below.
#include "classes/CGolfDoc.h"
#include "fp_fused.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/itemwindow.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CClock.h"
#include "classes/NameRegistry801C.h"
#include "classes/GolfDocSnapshot8013.h"
#include "iff/IffStruct.h"
#include "game/golfball.h"
#include "classes/CClub.h"
#include "classes/CQuadTree.h"
// GolfBall__Fv returns the physics CGolfBall; original ground type +0x59,
// power flags +0x44 and shot-spin setting +0x38 (shared CGolfBall layout in golfball.h).

#define AsCClock(p) ((CClock*)(p))

// Club parameter records use yards, radians and launch-speed units; list order is the club index.
// Distance tiers classify horizontal range in yards; these values also select
// the green-putter policy and wedge launch-speed formula.
enum ClubDistanceTier {
    kDistanceBelow10Yards = 0,
    kDistance10To15Yards = 1,
    kDistance15To28Yards = 2,
    kDistance28To58Yards = 3,
    kDistanceAtLeast58Yards = 4
};
// Unidentified terrain codes are named by this selector's observed score policy.
enum ClubTerrainPenaltyCode {
    kPenalty100Ground4 = 4,
    kUnpenalizedGround5 = 5,
    kPenalty100Ground6 = 6,
    kPenalty100Ground7 = 7,
    kUnpenalizedGround8 = 8,
    kPenalty100Ground10 = 10
};
enum ClubConstants {
    kClubWood = 0,
    kClubIron = 1,
    kClubWedge = 2,
    kClubPutter = 3,
    kFirstWedgeIndex = 11,
    kSandWedgeIndex = 12,
    kPutterIndex = 13,
    kClubNameCount = 17,
    kDistanceTierCount = 5,
    kAnimationDelay = 0,
    kAnimationSlideOut = 1,
    kAnimationReady = 2,
    kAnimationFadeIn = 3,
    kSelectionChanged = 1,
    kClubGroupChanged = 2,
    kSelectionChangeMask = 3,
    kPartyMode = 3,
    kClubLockItem = 11,
    kActionPreviousClub = 0,
    kActionNextClub = 1,
    kActionCameraModifier = 0x19,
    kCameraDefault = 0,
    kCameraClubSelection = 4,
    kClockStart = 0,
    kClockStop = 1,
    kClockReset = 0x2719,
    kClockMessageUpdate = 8,
    kClockMessageTimeout = 7,
    kTimeoutFlag = 0x4000,
    kWarningTimeMs = 10000,
    kClockBlueTimeMs = 20000,
    kClockYellowTimeMs = 15000,
    kClockRedTimeMs = 5000,
    kClockBlendTimeMs = 5000,
    kClockBlue = 0xFF147EFF,
    kClockYellow = 0xFFF014FF,
    kClockRed = 0xFFFF1450,
    kColorByteMask = 0xFF00FF00,
    kGroundTee = 0,
    kGroundGreen = 2,
    kGroundBunker = 3,
    kGroundWater = 9,
    kPlayerSlotBytes = 0x1D4,
    kPlayerSlotsOffset = 0x44,
    kInventoryPointerOffset = 0x1CC,
    kPowerStatOffset = 0xD8,
    kControlStatOffset = 0xDC,
    kHashFirstMask = 0x2A,
    kHashLastMask = 0x15,
    kHashLengthShift = 6
};

// Layout shared by the selection registry and its animation/shot-parameter views.
struct ClubSelectionStorage {
    void* vtable;
    void* selected;
    void* first;
    void* cursor;
    void* blocks;
    void* freeNodes;
    int nodesPerBlock;
    unsigned int hashMask;
    int bucketCount;
    void** buckets;
    char mutex[0x18];
    unsigned char clubIndex, minimumClubIndex;
    char padding42[2];
    int powerMode;
    unsigned char distanceTier;
    char padding49[3];
    unsigned int changeFlags;
    unsigned char longPutt;
    char padding51[3];
    float fadeSeconds;
    float opacity;
    float slidePosition;
    int animationState;
    unsigned char selectionFlag;
    unsigned char previousClubIndex;
    char padding66[2];
    float controlValue;
    float controlLimit;
    float powerBonus;
    unsigned char resourcesLoaded;
    char padding75[3];
    int inputState;
    int resourceCount;
    void* resourceNext;
    void* resourcePrevious;
};
struct ClubRegistryStorage {
    char prefix[8];
    NameRegistry801C registry;
};
struct ClubRegistryNodeView {
    void* data;
    const char* name;
    char links[0xC];
    void* bucketNext;
};
struct ClubParameterView {
    float distanceYards, launchAngle;
    unsigned char parameterKind, clubIndex, clubGroup, padding0B;
    float launchSpeed, selectionScale, selectionBias;
    char name[4];
    float selectionWeight;
    float preferenceBias;
};
struct ShotAdjustmentState {
    char prefix[0x10];
    float powerBonusRate;
    float impactWidthBonus;
    char padding18[0x24];
    float impactWidthDivisor;
    char padding40[0x20];
    int clockSuspended;
};
struct PlayerShotStats {
    char prefix[0xD8];
    int power;
    int control;
};
struct PlayerInventorySlot {
    char prefix[kInventoryPointerOffset];
    void* inventory;
};
struct PlayerRankSlot {
    void* rank;
    char rest[kPlayerSlotBytes - 4];
};
struct SharedRankSlots {
    char prefix[kPlayerSlotsOffset];
    PlayerRankSlot players[4];
};
struct ClockMessage {
    int sender;
    int type;
};
struct ClubCachedResourceSet {
    char prefix[4];
    void* resourcesA[4];
    void* resourcesB[4];
};
struct ClubCachedResourceNode {
    void* next;
    void* previous;
    void* resources;
};

// Original Shift-JIS string pools, described in place without new storage.
struct ClockStrings {
    char screen[7];
    char timeSound[7];
    char golfRule[9];
    char className[7];
    char alignmentPadding[2];
};
struct ClubStrings {
    char wood1[3], wood2[3], wood3[3];
    char iron2[3], iron3[3], iron4[3], iron5[3], iron6[3], iron7[3], iron8[3], iron9[3];
    char pitchingWedge[3], sandWedge[3], shortPutter[4], longPutter[4];
    char leftAlt[5], rightAlt[5], player[7], unplayableSound[16], screen[7], changeSound[11];
    char petFormat[7], camera[7], target[7], angle[7], floatFormat[3];
    char alignmentPadding[7];
};
#define CLUB_FIELD_OFFSET(field) ((int)&((ClubSelectionStorage*)0)->field)
#define CLUB_STRING_OFFSET(field) ((int)&((ClubStrings*)0)->field)
#define CLOCK_STRING_OFFSET(field) ((int)&((ClockStrings*)0)->field)
#define CLOCK_FIELD_OFFSET(field) ((int)&((CClock*)0)->field)

extern "C" {

extern void* s_clockTypeInfo;
extern void* s_actorTypeInfo;
extern const float kFloat_6_2831855;
extern float s_clockRegistrationTwoPi;
extern ClubStrings clubAndPlayerStrings;
extern const float kFloat_230_Club;
extern const float kFloat_10_GolfHud;
extern const float kFloat_236_Club;
extern const float kFloat_1_61_Club;
extern const float kFloat_0_55_Club;
extern const float kFloat_210_ClubSelection;
extern const float kFloat_13_Club;
extern const float kFloat_204_ClubSelection;
extern const float kFloat_1_41_Club;
extern const float kFloat_0_5_Camera;
extern const float kFloat_190_Club;
extern const float kFloat_16_Club;
extern const float kFloat_176_Club;
extern const float kFloat_1_26_Club;
extern const float kFloat_0_45_Club;
extern const float kFloat_180_ClubSelection;
extern const float kFloat_20_Club;
extern const float kFloat_161_Club;
extern const float kFloat_1_07_ClubSelection;
extern const float kFloat_170_Club;
extern const float kFloat_24_Club;
extern const float kFloat_149_Club;
extern const float kFloat_0_95_Club;
extern const float kFloat_160_Club;
extern const float kFloat_28_GolfDocHelpers;
extern const float kFloat_139_Club;
extern const float kFloat_0_83_ClubSelection;
extern const float kFloat_150_Club;
extern const float kFloat_32_ClubSelection;
extern const float kFloat_131_Club;
extern const float kFloat_0_73_Club;
extern const float kFloat_140_Club;
extern const float kFloat_36_Club;
extern const float kFloat_124_Club;
extern const float kFloat_0_67_Club;
extern const float kFloat_0_41_ClubSelection;
extern const float kFloat_130_Club;
extern const float kFloat_40_Club;
extern const float kFloat_118_ClubSelection;
extern const float kFloat_0_61_Club;
extern const float kFloat_0_36_Club;
extern const float kFloat_120_Club;
extern const float kFloat_44_Club;
extern const float kFloat_114_Club;
extern const float kFloat_0_57_Club;
extern const float kFloat_0_3_ClubSelection;
extern const float kFloat_110_Club;
extern const float kFloat_48_Club;
extern const float kFloat_0_53_ClubSelection;
extern const float kFloat_0_25_Club;
extern const float kFloat_100_Club;
extern const float kFloat_52_Club;
extern const float kFloat_107_Club;
extern const float kFloat_0_49_Club;
extern const float kFloat_0_18_Club;
extern const float kFloat_80_ClubSelection;
extern const float kFloat_56_Club;
extern const float kFloat_93_ClubSelection;
extern const float kFloat_0_42_Club;
extern const float kFloat_0_17_ClubSelection;
extern const float kFloat_0_Club;
extern const float kFloat_30_Club;
extern const float kFloat_21_Club;
extern unsigned int g_resourceManager;
extern void* g_sharedDoc;
extern void* g_soundManager;
extern float kFloat_1000_GolfDocHelpers;
extern float kFloat_0_017453292_Clock;
extern float kFloat_12_Clock;
extern float kFloat_278_Clock;
extern float kFloat_1_Clock;
extern float kFloat_0_5_Clock;
extern float kFloat_200_Clock;
extern const float kFloat_0_20943952_Clock;
extern void* g_inputPointer;
extern signed char s_inputManagerInitialized;
extern void* g_taskManager;
extern void* g_helpMsg;
extern unsigned char s_gameInputManager[];
unsigned char s_inputManagerDtorRegistration_Club[0xC];
extern const float kFloat_200_Club;
extern const float kFloat_1_golfParameters;
extern const float kFloat_0_7_Club;
extern const float kFloat_456_Club;
extern const float kFloat_1000_Club;
extern const float kFloat_765_ClubSelection;
extern const float kFloat_255_Club;
extern const float kGolfDegreesToRadians;
// Club names in parameter-kind order, including the two unregistered woods.
extern const char* s_clubParameterKindNames[];
// Selection preference biases indexed by the name table above.
extern float s_clubParameterPreferenceBiases[];
extern const float kFloat_3_2_Club;
extern const float kFloat_15_Club;
extern const float kFloat_58_Club;
extern const float kFloat_35_GolfHud;
extern const float kFloat_neg10000_ClubSelection;
extern const float kFloat_0_8_Club;
extern const float kFloat_0_1_ClubTrajectory;
extern const float kFloat_1_01_ClubSampling;
extern const float kFloat_0_06981317_GolfDocHelpers;
extern const float kFloat_neg1_ClubSelection;
extern float s_clubGroundTypeScoreOffsets[];
extern const float kFloat_60_Club;
extern const float kFloat_0_6_Club;
extern const float kFloat_2_golfParameters;
extern const float kFloat_1_5_GolfParameters;
extern const float kFloat_1_3_GolfParameter;
extern const float kWedgeSpeedReferenceAngleRadians;
extern const float kWedgeMidrangePowerSpeedAddition;
extern const float kFloat_42_Club;
extern Arr3 s_golfParameterModeOffsets;

void* GolfBall__Fv();

extern const char kClockActorFactoryName[7];
extern const char kClockTaskFactoryName[7];
extern const char clockMessageStrings[];

void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
int GolfTask_NoOpStatus();
void CClock_Create();
void appFreeUni__FPv(void*);
void IActor_DispatchMessage(void*, void*, const char*, int, unsigned int, void*, int, int);
void Sound_StopByAlias(void*, const char*);
void ResourceManager_ReleaseObject(void*, void*, int);
void* appMallocUni__FUl(unsigned int);
struct HashManager;
struct TreeLinks {
    TreeLinks* next;
    TreeLinks* prev;
};
struct WInstanceLock {
    void* lock;
    // Acquire the registry mutex for this lexical scope.
    WInstanceLock(void* p) : lock(p) {
        OSLockMutex((OSMutex*)p);
    }
    // Release the registry mutex on normal or exceptional scope exit.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
};

// Hash a name under the registry mutex and mask it to the bucket count.
inline unsigned int EA88CHash(void* self, char* key) {
    unsigned int pick;
    void* mutex = ((NameRegistry801C*)self)->lock;
    OSLockMutex((OSMutex*)mutex);
    size_t length = strlen(key);
    long signedLength = (long)length;
    unsigned char first = key[0] & 0x2a;
    pick = signedLength > 0 ? length - 1 : 0;
    unsigned char second = key[pick] & 0x15;
    unsigned int hash = (length << 6) | (first | second);
    unsigned int bucket = hash & ((NameRegistry801C*)self)->hashMask;
    OSUnlockMutex((OSMutex*)mutex);
    return bucket;
}

// Return the club record found in the locked name registry, or null.
inline void* EA88CLookup(void* self, char* key) {
    void* result;
    TreeLinks* node;
    WInstanceLock lock(((ClubRegistryStorage*)self)->registry.lock);
    if (key == 0 || ((ClubRegistryStorage*)self)->registry.buckets == 0) {
        result = 0;
        return result;
    }
    unsigned int bucket = EA88CHash(&((ClubRegistryStorage*)self)->registry, key);
    node = ((TreeLinks**)((ClubRegistryStorage*)self)->registry.buckets)[bucket];
    while (node != 0) {
        if (strcmp(((ClubRegistryNodeView*)node)->name, key) == 0) {
            result = node->next;
            return result;
        }
        node = (TreeLinks*)((ClubRegistryNodeView*)node)->bucketNext;
    }
    result = 0;
    return result;
}

struct EA420Node {
    void* data;
    char pad[8];
    EA420Node* prev;
    EA420Node* next;
};
// Return the next registered club record and advance the circular-list cursor.
inline void* EA420Read(void* mutex, char* base) {
    void* data;
    WInstanceLock lock(mutex);
    EA420Node* node = *(EA420Node**)(base + CLUB_FIELD_OFFSET(cursor));
    if (node != 0) {
        EA420Node* next = node->next;
        data = node->data;
        if (next == *(EA420Node**)(base + CLUB_FIELD_OFFSET(first)))
            next = 0;
        *(EA420Node**)(base + CLUB_FIELD_OFFSET(cursor)) = next;
        return data;
    }
    return 0;
}
// Unlink one node from the circular registry list and return its successor.
inline EA420Node* EA420Remove(void* mutex, EA420Node* node) {
    EA420Node* next;
    WInstanceLock lock(mutex);
    node->next->prev = node->prev;
    node->prev->next = node->next;
    next = node->next;
    if (next == node)
        next = 0;
    return next;
}
// Insert a recycled node before the circular list head, or create a one-node list.
inline EA420Node* EA420Insert(void* mutex, EA420Node* tail, EA420Node* current) {
    WInstanceLock lock(mutex);
    if (tail == 0) {
        current->prev = current;
        current->next = current;
        return current;
    }
    current->next = tail;
    current->prev = tail->prev;
    tail->prev = current;
    current->prev->next = current;
    return tail;
}

void* PartyItemSlots_GetInstance();
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);

extern const char s_cameraTransform[];
struct ClubInputState;
struct ClubAnimationState;
void Matrix_CopyAffine(const char*, void*);

void InstanceTable_ClearCachedResources(void* self);

void* InstanceTable_Get();

void* __dt__6CClockFv(void* self, int shouldDelete);

void CClock_RegisterType();

int Pointer_IsActionActive(void*, const char*, int);
int Input_IsActionButtonDown(void*, int, int);
int Input_IsActionPressed(void*, int, int);
void InputManager_Init(void*);
void InputManager_Dtor();

int Object_GetChild(void*);
void* TaskActorTable_FindByName(int, const char*);

int PartyItemSlots_GetItemType(int, int);

void NoticeQueue_ShowById(void*, int, int);

void* __nw__FUl(unsigned int);

int strcmp(const char*, const char*);
char* strcpy(char*, const char*);

struct EANode {
    unsigned char* value;
    char pad[12];
    EANode* next;
};
struct ClubInputState {
    void* vtable;
    unsigned char* selected;
    EANode* first;
    EANode* cursor;
    char pad10[0x18];
    int mutex[6];
    unsigned char clubIndex, minimumClubIndex;
    char pad42[2];
    int powerMode;
    char pad48[0x30];
    int inputState;
};
struct EAMessage {
    int zero, code;
    int* result;
    int rest[5];
    // Construct a zero-initialized message carrying a command and borrowed result pointer.
    EAMessage(int c, int* r) {
        zero = 0;
        code = c;
        result = r;
        rest[0] = 0;
        rest[1] = 0;
        rest[2] = 0;
        rest[3] = 0;
        rest[4] = 0;
    }
};
struct EAReceiver {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void f5();
    virtual void f6();
    virtual void f7();
    virtual void f8();
    virtual void send(EAMessage*);
};
// Initialize and register the input-manager singleton on first use.
static void EAInput() {
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (char*)InputManager_Dtor, s_inputManagerDtorRegistration_Club);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
}
// Return the next club record and stop after one traversal of the circular list.
static unsigned char* EANext(ClubInputState* self) {
    unsigned char* value;
    int* mutex = self->mutex;
    OSLockMutex((OSMutex*)mutex);
    EANode* node = self->cursor;
    if (node) {
        EANode* next = node->next;
        value = node->value;
        if (next == self->first)
            next = 0;
        self->cursor = next;
        OSUnlockMutex((OSMutex*)mutex);
        return value;
    }
    OSUnlockMutex((OSMutex*)mutex);
    return 0;
}
// Reset the club-list cursor and return the first record.
static unsigned char* EAFirst(ClubInputState* self) {
    WInstanceLock lock(self->mutex);
    self->cursor = self->first;
    return EANext(self);
}
// The byte-sized loop and caller-local result preserve CodeWarrior register allocation.
#define SELECT_CLUB_AT_INDEX(result, position)                       \
    unsigned char* result = EAFirst(self);                           \
    for (unsigned char step = 0; step < (unsigned int)(position);) { \
        ++step;                                                      \
        result = EANext(self);                                       \
    }

// Registry view map (all offsets are receiver-relative):
// ClubSelectionStorage owns registry + selection/UI state. HashManager is its
// registry-only view (+08 first, +1c mask, +24 buckets, +28 mutex); input,
// animation, lookup and ClubSelectionState views expose subsets of this receiver.
// HashNode/ItemEntry/EANode are node views, never parameter records: +00 payload,
// +04 name, +0c/+10 active-list links, +14 next hash-bucket node.
// Entry/ClubParameterView/ClubParameterPrefix are the PAYLOAD at node+00:
// +00 distance yards, +04 launch radians, +09 list index, +0a club group,
// +0c launch speed, +18 name, +1c selection weight, +20 preference bias.
// Entry's union overlays name bytes and the weight/bias tail, preserving the
// recovered short-name representation; do not extend its name independently.
struct HashNode {
    void* data;          // 0x00
    const char* key;     // 0x04
    unsigned char kind;  // 0x08
    unsigned char count; // 0x09
    unsigned char pad0A[2];
    HashNode* next;       // 0x0C
    HashNode* prev;       // 0x10
    HashNode* bucketNext; // 0x14
};
struct Entry {
    float distanceYards;     // 0x00
    float launchAngle;       // 0x04
    unsigned char kind;      // 0x08
    unsigned char listIndex; // 0x09
    unsigned char clubGroup; // 0x0A: club GROUP, not listIndex
    unsigned char pad0B;
    float launchSpeed;    // 0x0C
    float selectionScale; // 0x10
    float selectionBias;  // 0x14
    union {
        char name[12]; // 0x18
        struct {
            char pad[4];
            float scale;
            float preferenceBias;
        } tail;
    } u;
};
struct HashManager {
    unsigned char pad00[8];
    HashNode* first; // 0x08
    unsigned char pad0C[4];
    HashNode* freeFirst; // 0x10
    HashNode* freeNext;  // 0x14
    int capacity;        // 0x18
    unsigned int mask;   // 0x1C
    unsigned char pad20[4];
    HashNode** buckets;       // 0x24
    unsigned char lock[0x28]; // 0x28
};
// Insert a hash node after the circular-list head while holding its mutex.
inline HashNode* HashPushAfter(void* lock, HashNode* head, HashNode* node) {
    void* lk = lock;
    OSLockMutex((OSMutex*)lk);
    if (!head) {
        node->next = node;
        node->prev = node;
        OSUnlockMutex((OSMutex*)lk);
        return node;
    }
    node->prev = head;
    node->next = head->next;
    head->next = node;
    node->next->prev = node;
    OSUnlockMutex((OSMutex*)lk);
    return head;
}
// Unlink a hash node and return the preceding node, or null for a one-node list.
inline HashNode* HashDetach(void* lock, HashNode* n) {
    HashNode* nx;
    void* lk = lock;
    OSLockMutex((OSMutex*)lk);
    n->prev->next = n->next;
    n->next->prev = n->prev;
    nx = n->prev;
    if (nx == n)
        nx = 0;
    OSUnlockMutex((OSMutex*)lk);
    return nx;
}
// Hash club names from length and selected bits of their first and last bytes.
static unsigned int EAHash(const unsigned char* s, HashManager* self) {
    int len = strlen((const char*)s);
    int last = len > 0 ? len - 1 : 0;
    unsigned raw = (len << 6) | ((s[0] & 0x2a) | (s[last] & 0x15));
    return raw & self->mask;
}
// Compute a club-name bucket index while holding the hash mutex.
static inline unsigned int HashKey(HashManager* self, const char* name) {
    unsigned int h;
    void* lk = &self->lock;
    OSLockMutex((OSMutex*)lk);
    h = EAHash((const unsigned char*)name, self);
    OSUnlockMutex((OSMutex*)lk);
    return h;
}
// Look up a named club parameter record in the hash bucket chain.
inline Entry* HashFind(HashManager* self, const char* name) {
    int hash;
    HashNode* node;
    WInstanceLock lock(&self->lock);
    if (!name || !self->buckets)
        return 0;
    hash = HashKey(self, name);
    node = self->buckets[hash];
    while (node) {
        if (!strcmp(node->key, name))
            return (Entry*)node->data;
        node = node->bucketNext;
    }
    return 0;
}
// Acquire a recycled hash node, attach its club record and link the bucket and active list.
inline void HashInsert(HashManager* self, Entry* entry) {
    HashNode* head;
    const char* key;
    int hash;
    HashNode* node;
    key = entry->u.name;
    WInstanceLock lockA(&self->lock);
    {
        WInstanceLock lockB(&self->lock);
        if (!self->freeNext) {
            HashNode* nodes = (HashNode*)appMallocUni__FUl(self->capacity * 0x18);
            self->freeFirst = HashPushAfter(&self->lock, self->freeFirst, nodes);
            for (int i = 1; self->capacity > i; ++i)
                self->freeNext = HashPushAfter(&self->lock, self->freeNext, &nodes[i]);
        }
        node = self->freeNext;
        self->freeNext = HashDetach(&self->lock, node);
    }
    node->data = entry;
    node->kind = 0;
    node->key = key;
    if (key) {
        void* lk = &self->lock;
        {
            OSLockMutex((OSMutex*)lk);
            hash = EAHash((const unsigned char*)key, self);
            OSUnlockMutex((OSMutex*)lk);
        }
        {
            OSLockMutex((OSMutex*)lk);
            node->bucketNext = self->buckets[hash];
            self->buckets[hash] = node;
            OSUnlockMutex((OSMutex*)lk);
        }
    }
    head = self->first;
    self->first = HashPushAfter(&self->lock, head, node);
}

struct ClubAnimationState {
    unsigned char pad[0x4C];
    unsigned int flags;
    float unk_50;
    float fadeSeconds;
    float opacity;
    float slidePosition;
    int animationState;
};

typedef Vec3f Vec3;
struct Vec3i {
    unsigned int x, y, z;
};

struct ClubParameterPrefix;

struct ItemEntry {
    ClubParameterPrefix* child; // 0x00
    const char* name;           // 0x04
    unsigned char pad08[8];     // 0x08
    ItemEntry* next;            // 0x10
    ItemEntry* hashNext;        // 0x14
};

struct ClubParameterPrefix {
    unsigned char pad00[4];
    float launchAngle;       // 0x04
    unsigned char pad08;     // 0x08
    unsigned char clubIndex; // 0x09
    unsigned char pad0a[6];  // 0x0a
    unsigned char pad10[8];  // 0x10
    struct {
        char name[4];
        float weight;
        float bias;
    } selection; // 0x18
};

struct ClubSelectionState {
    unsigned char pad00[4];        // 0x00
    ClubParameterPrefix* selected; // 0x04
    ItemEntry* first;              // 0x08
    ItemEntry* current;            // 0x0c
    unsigned char pad10[0x0c];
    unsigned int mask; // 0x1c
    unsigned char pad20[4];
    ItemEntry** buckets;       // 0x24
    unsigned char mutex[0x18]; // 0x28
    unsigned char clubIndex;   // 0x40
    // 0x41: mixed selection state: counts slope rejects off-green, forced to 13 on
    // green and floored at 7 for short approaches. Final consumer meaning unresolved.
    // The field name is provisional.
    unsigned char selectionPolicyValue;
    unsigned char pad42[6];
    unsigned char distanceTier; // 0x48
    unsigned char pad49[7];
    unsigned char longPutt; // 0x50
    unsigned char pad51[0x14];
    unsigned char previousClubIndex; // 0x65
    unsigned char pad66[2];
    float controlValue; // 0x68
    float controlLimit; // 0x6c
};

struct PlayerControlStatsPrefix {
    char prefix[kControlStatOffset];
    unsigned int control;
};

void __mi__FRC7WVectorRC7WVector(Vec3*, void*, void*);
void __pl__FRC7WVectorRC7WVector(Vec3*, void*, Vec3*);
void __ml__FfRC7WVector(Vec3*, Vec3*, float);
float Magnitude__7WVectorCFv(Vec3*);
float Normalize__7WVectorFv(Vec3*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* CQuadTree_GetInstance();
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, Vec3*);
int CourseData_IsPointOutsidePlayableRegion(void*, Vec3*);
int PlayerRank_FindTierForValue(void*);
void GolfAim_FindTargetAlongPath(void*, void*, const char*);
float GolfParameter_ResolveFloat(void*, void*);

// Search the locked club-name hash; a single-pass block handles missing names and buckets.
// Successful matches retain their separate mutex-cleanup join; inline-return helpers changed code.
// A fresh output-reference inline-helper trial also changed .text/relocations.
// Contract: receiver is a ClubSelectionState*; keyexpr is evaluated once; found receives a borrowed
// registry record (or null). n supplies a unique cleanup-join label, not a club ID.
// The recursive registry mutex spans hash calculation AND bucket traversal; the
// explicit inner lock protects the hash calculation too. Keep both lock levels.
#define FIND_CLUB_BY_NAME(receiver, keyexpr, found, n)                                                           \
    do {                                                                                                         \
        const char* clubName = (keyexpr);                                                                        \
        ClubParameterPrefix* foundClub;                                                                          \
        do {                                                                                                     \
            {                                                                                                    \
                WInstanceLock registryLock(&(receiver)->mutex[0]);                                               \
                if (!clubName || !(receiver)->buckets)                                                           \
                    break;                                                                                       \
                OSLockMutex((OSMutex*)((receiver)->mutex));                                                      \
                unsigned int nameLength = strlen(clubName);                                                      \
                unsigned int lastIndex = (int)nameLength > 0 ? nameLength - 1 : 0;                               \
                unsigned int bucketIndex = ((nameLength << 6) | (((unsigned char)clubName[0] & 0x2a) |           \
                                                                 ((unsigned char)clubName[lastIndex] & 0x15))) & \
                                           (receiver)->mask;                                                     \
                OSUnlockMutex((OSMutex*)((receiver)->mutex));                                                    \
                ItemEntry* entry = (receiver)->buckets[bucketIndex];                                             \
                while (entry) {                                                                                  \
                    if (!strcmp(entry->name, clubName)) {                                                        \
                        foundClub = entry->child;                                                                \
                        goto found_##n;                                                                          \
                    }                                                                                            \
                    entry = entry->hashNext;                                                                     \
                }                                                                                                \
            }                                                                                                    \
        } while (0);                                                                                             \
        foundClub = 0;                                                                                           \
        found_##n : (found) = foundClub;                                                                         \
    } while (0)

// Advance the selected-club list cursor while holding its mutex.
inline ClubParameterPrefix* AdvanceTarget(ClubSelectionState* s) {
    OSMutex* mutex = (OSMutex*)(s->mutex);
    OSLockMutex((OSMutex*)mutex);
    ItemEntry* c = s->current;
    if (c) {
        ItemEntry* next = c->next;
        ClubParameterPrefix* child = c->child;
        if (next == s->first)
            next = 0;
        s->current = next;
        OSUnlockMutex((OSMutex*)mutex);
        return child;
    } else {
        OSUnlockMutex((OSMutex*)mutex);
        return 0;
    }
}
#define ADVANCE(receiver, dst) ((dst) = AdvanceTarget(receiver))

// CPU lookup has the same search, with its own mutex temporary for the original instruction schedule.
#define FIND_CPU_CLUB_BY_NAME(keyexpr, found, n)                                                                 \
    do {                                                                                                         \
        const char* clubName = (keyexpr);                                                                        \
        ClubParameterPrefix* foundClub;                                                                          \
        do {                                                                                                     \
            {                                                                                                    \
                WInstanceLock registryLock(&s->mutex[0]);                                                        \
                if (!clubName || !s->buckets)                                                                    \
                    break;                                                                                       \
                OSMutex* registryMutex = (OSMutex*)(s->mutex);                                                   \
                OSLockMutex((OSMutex*)registryMutex);                                                            \
                unsigned int nameLength = strlen(clubName);                                                      \
                unsigned int lastIndex = (int)nameLength > 0 ? nameLength - 1 : 0;                               \
                unsigned int bucketIndex = ((nameLength << 6) | (((unsigned char)clubName[0] & 0x2a) |           \
                                                                 ((unsigned char)clubName[lastIndex] & 0x15))) & \
                                           s->mask;                                                              \
                OSUnlockMutex((OSMutex*)registryMutex);                                                          \
                ItemEntry* entry = s->buckets[bucketIndex];                                                      \
                while (entry) {                                                                                  \
                    if (!strcmp(entry->name, clubName)) {                                                        \
                        foundClub = entry->child;                                                                \
                        goto found_##n;                                                                          \
                    }                                                                                            \
                    entry = entry->hashNext;                                                                     \
                }                                                                                                \
            }                                                                                                    \
        } while (0);                                                                                             \
        foundClub = 0;                                                                                           \
        found_##n : (found) = foundClub;                                                                         \
    } while (0)
void ClubSelection_SelectForCpuTarget(ClubSelectionState* self, void* arg);

float CSharedDoc_ComputeAuxPartModifier(void*, unsigned char, int);
int ItemInventory_IsSlot14ClubSetId(void*);

unsigned int ItemInventory_GetItemIdAt(void*, int);

} // extern "C"

// The instance table @LOCAL@GetInstanceTable__Fv@s is a real class: its hash and list members are destroyed if the body's
// CClub::Init throws (the game's exception table has both). ~TableHash is __dt__9TableHashFv, ~TableList __dt__9TableListFv.
struct TableMutex {
    int data[0x24 / 4];
    TableMutex() {
        OSInitMutex((OSMutex*)this);
    }
};
struct TableHash {
    int f00, f04, f08, f0C, f10, mask, count;
    void** buckets;
    TableMutex mutex;
    TableHash();
    ~TableHash();
};
// Initialize empty hash storage, its mutex and initial buckets.
inline TableHash::TableHash() : buckets(0) {
    WInstanceLock lock(&mutex);
    f00 = 0;
    f0C = 0;
    f08 = 0;
    f10 = 0x10;
    f04 = 0;
    count = 8;
    if (count > 0) {
        buckets = (void**)appMallocUni__FUl(0x20);
        mask = count - 1;
        for (int i = 0; i < count; ++i)
            buckets[i] = 0;
    } else {
        buckets = 0;
    }
}
struct TableList {
    int f00;
    void* next;
    void* prev;
    // Construct an empty circular list whose links point to its embedded sentinel.
    TableList() : f00(0) {
        void* end = &next;
        prev = end;
        next = end;
    }
    ~TableList();
};
struct InstanceTableReal {
    char padding00[4];
    int f04;
    TableHash hash;
    int f4C;
    char padding50[0x2C];
    TableList list;
    ~InstanceTableReal();
    // Initialize the table members and register default club parameters.
    InstanceTableReal() : f04(0), f4C(0) {
        ((CClub*)this)->Init();
    }
};

// Construct the club parameter table once and return its process-lifetime instance.
inline InstanceTableReal* GetInstanceTable() {
    static InstanceTableReal s;
    return &s;
}

extern "C" {

struct TableLayout {
    char padding00[4];
    int f04, f08, f0C, f10, f14, f18, mask, count;
    void** buckets;
    int lockData[0x24 / 4];
    int f4C;
    char padding50[0x2C];
    int f7C;
    void* f80;
    void* f84;
};

struct LockRef {
    void* lock;
    // Acquire a mutex that this helper requires the caller to release explicitly.
    LockRef(void* p) : lock(p) {
        OSLockMutex((OSMutex*)p);
    }
    // Release the explicitly managed mutex once.
    void release() {
        OSUnlockMutex((OSMutex*)lock);
    }
    // Leave unlocking to release(); this destructor intentionally performs no work.
    ~LockRef() {}
};

struct EntryNode {
    EntryNode* payload;
    char padding04[0x0C];
    EntryNode* same;
    char padding14[0x10];
};

struct LookupEntryNode {
    LookupEntryNode* payload;
    const char* key;
    char padding08[0x0C];
    LookupEntryNode* same;
    char padding14[0x10];
};

struct LookupState {
    char padding00[4];
    EntryNode* result;
    EntryNode* end;
    EntryNode* current;
    char padding10[0x0C];
    unsigned int hashMask;
    unsigned int pad20;
    unsigned int* buckets;
    char lock[4];
    char padding2C[0x14];
    unsigned char flag;
    // Return the next borrowed payload under the supplied mutex, stopping at the sentinel.
    EntryNode* nextNodeAt(void* lk) {
        WInstanceLock guard(lk);
        EntryNode* cursor = current;
        if (cursor != 0) {
            EntryNode* next = cursor->same;
            EntryNode* item = cursor->payload;
            if (next == end)
                next = 0;
            current = next;
            return item;
        }
        return 0;
    }
    // Return the next borrowed payload under this lookup view's mutex.
    EntryNode* nextNode() {
        WInstanceLock guard(lock);
        EntryNode* cursor = current;
        if (cursor != 0) {
            EntryNode* next = cursor->same;
            EntryNode* item = cursor->payload;
            if (next == end)
                next = 0;
            current = next;
            return item;
        }
        return 0;
    }
};

struct ProgressState {
    char padding00[0x68];
    float progress;
    float limit;
};

struct ProgressPayload {
    char padding00[0x1C];
    float value;
};

struct ProgressNode {
    ProgressPayload* item;
    char padding04[0x0C];
    ProgressNode* next;
};

struct ProgressList {
    char padding00[8];
    ProgressNode* first;
    ProgressNode* current;
    char padding10[0x18];
    char lock[4];
    // Return a progress payload and advance one circular-list step under its mutex.
    ProgressPayload* nextItem() {
        WInstanceLock guard(lock);
        ProgressNode* cursor = current;
        if (cursor != 0) {
            ProgressNode* next = cursor->next;
            ProgressPayload* item = cursor->item;
            if (next == first)
                next = 0;
            current = next;
            return item;
        }
        return 0;
    }
};

struct ValueNode {
    ValueNode* payload;
    const char* key;
    char padding08[0x0C];
    ValueNode* same;
    char padding18[4];
    float unused;
};

struct ValueLookup {
    char padding00[4];
    ValueNode* fallback;
    char padding08[0x14];
    unsigned int hashMask;
    char padding20[4];
    ValueNode** buckets;
    char lock[4];
    char padding2C[0x1C];
    unsigned char mode;
};

struct TargetLookup {
    char pad[0x1C];
    unsigned int hashMask;
    char pad20[8];
    char mutex[4];
    // Hash a non-null club name into a bucket under the recursive registry mutex.
    unsigned int hash(const char* key) {
        void* lock = mutex;
        OSLockMutex((OSMutex*)lock);
        int length = strlen(key);
        unsigned int first = (unsigned char)key[0] & 0x2A;
        int lastIndex = length > 0 ? length - 1 : 0;
        unsigned int last = (unsigned char)key[lastIndex] & 0x15;
        unsigned int index = (first | last | ((unsigned int)length << 6)) & hashMask;
        OSUnlockMutex((OSMutex*)lock);
        return index;
    }
    // Search a locked name bucket and return its borrowed payload or null.
    void* find(const char* key) {
        void* self = this;
        WInstanceLock outer(((ClubSelectionStorage*)self)->mutex);
        if (key == 0 || ((ClubSelectionStorage*)self)->buckets == 0) {
            return 0;
        } else {
            unsigned int index = hash(key);
            void* node = ((void**)((ClubSelectionStorage*)self)->buckets)[index];
            while (node != 0) {
                if (strcmp(((ClubRegistryNodeView*)node)->name, key) == 0) {
                    return *(void**)node;
                }
                node = ((ClubRegistryNodeView*)node)->bucketNext;
            }
            return 0;
        }
    }
};

float ShotParameters_GetBaseLaunchSpeed(void* self);

float ShotParameters_GetGoodImpactHalfWidth(void* self);

float ShotParameters_GetPoorImpactHalfWidth(void* self);

float ClubSelection_GetLaunchAngle(ValueLookup* self, const char* key);

void ShotProgressList_SetAllValuesToOne(ProgressList* self);

void ShotParameters_UpdateControlWindow(ProgressState* self);

void ClubSelection_SetClubIndex(LookupState* self, unsigned int count);

void ClubSelection_SelectByName(LookupState* self, const char* key);

// Advance elapsed/countdown time, blend the clock color, play warnings and dispatch a timeout when time expires.
void CClock::OnProcess(float delta) {
    char* obj = (char*)this;
    {
        char* state = (char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(state)->unk_35D || AsCGolfDoc(state)->unk_3CB)
            return;
        if (AsCGolfDoc(state)->tutorialStage < 6)
            return;
        AsCGolfDoc(state)->elapsedTimeMs += Runtime_ConvertDoubleToUnsigned(kFloat_1000_GolfDocHelpers * delta);
    }
    if (!AsCClock(obj)->running)
        return;
    if (((ShotAdjustmentState*)PartyItemSlots_GetInstance())->clockSuspended != 0) {
        IActor_DispatchMessage(this, this, clockMessageStrings + CLOCK_STRING_OFFSET(golfRule), 8, 0, 0, 0, 0);
        return;
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs) == 0)
        return;
    {
        char* state = (char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(state)->remainingTimeMs > Runtime_ConvertDoubleToUnsigned(kFloat_1000_GolfDocHelpers * delta)) {
            AsCGolfDoc(state)->remainingTimeMs -= Runtime_ConvertDoubleToUnsigned(kFloat_1000_GolfDocHelpers * delta);
        } else {
            AsCGolfDoc(state)->remainingTimeMs = 0;
        }
    }
    unsigned int limitMs = AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs;
    unsigned int remainingMsForRatio = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->remainingTimeMs;
    float remainingRatio = (float)remainingMsForRatio / (float)limitMs;
    AsCClock(obj)->countdownDisplayValue =
        kFloat_0_017453292_Clock * (kFloat_12_Clock + kFloat_278_Clock * (kFloat_1_Clock - remainingRatio));

    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->remainingTimeMs < kWarningTimeMs &&
        !AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->shotPhase) {
        if (SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager,
                                                         clockMessageStrings + CLOCK_STRING_OFFSET(timeSound)) == 0)
            Sound_PlayByName(g_soundManager, clockMessageStrings + CLOCK_STRING_OFFSET(timeSound), 0, 1,
                             clockMessageStrings + CLOCK_STRING_OFFSET(timeSound), 0, kFloat_0_5_Clock,
                             kFloat_200_Clock, kFloat_1_Clock);
    }

    unsigned int remainingMs = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->remainingTimeMs;
    if (remainingMs > kClockBlueTimeMs)
        AsCClock(obj)->displayColor = kClockBlue;
    else if (remainingMs > kClockYellowTimeMs) {
        unsigned int complement;
        unsigned int blendAmount;
        blendAmount = (unsigned char)(((kClockBlueTimeMs - remainingMs) * 255) / kClockBlendTimeMs);
        complement = 255 - blendAmount;
        unsigned int c1 = complement * 0x1400FF + blendAmount * 0xF000FF;
        unsigned int c2 = complement * 0xFF007E + blendAmount * 0xFF0014;
        AsCClock(obj)->displayColor = (((c1 & kColorByteMask) >> 8) | (c2 & kColorByteMask));
    } else if (remainingMs > kWarningTimeMs)
        AsCClock(obj)->displayColor = kClockYellow;
    else if (remainingMs > kClockBlendTimeMs) {
        unsigned int complement;
        unsigned int blendAmount;
        blendAmount = (unsigned char)(((kWarningTimeMs - remainingMs) * 255) / kClockBlendTimeMs);
        complement = 255 - blendAmount;
        unsigned int c1 = complement * 0xF000FF + blendAmount * 0xFF0050;
        unsigned int c2 = complement * 0xFF0014 + blendAmount * 0xFF0014;
        AsCClock(obj)->displayColor = (((c1 & kColorByteMask) >> 8) | (c2 & kColorByteMask));
    } else
        AsCClock(obj)->displayColor = kClockRed;
    IActor_DispatchMessage(this, this, clockMessageStrings, 8, *(unsigned int*)(obj + CLOCK_FIELD_OFFSET(displayColor)),
                           obj + CLOCK_FIELD_OFFSET(countdownDisplayValue), 0, 0);
    {
        char* state = (char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(state)->shotPhase)
            return;
        if (AsCGolfDoc(state)->remainingTimeMs != 0)
            return;
        AsCClock(obj)->running = 0;
    }
    if (GolfTask_NoOpStatus())
        return;
    {
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->remainingTimeMs =
            AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs;
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->elapsedTimeMs = 0;
    }
    AsCClock(obj)->countdownDisplayValue = kFloat_0_20943952_Clock;
    *(unsigned int*)(obj + CLOCK_FIELD_OFFSET(displayColor)) = kClockBlue;
    AsCClock(obj)->running = 0;
    IActor_DispatchMessage(this, this, clockMessageStrings, 8, kClockBlue,
                           obj + CLOCK_FIELD_OFFSET(countdownDisplayValue), 0, 0);
    Sound_StopByAlias(g_soundManager, clockMessageStrings + CLOCK_STRING_OFFSET(timeSound));
    IActor_DispatchMessage(this, this, clockMessageStrings + CLOCK_STRING_OFFSET(golfRule), 7, 0x4000, 0, 0, 0);
}

#pragma opt_common_subs off
// Handle clock reset, start and stop messages while updating the displayed value and warning sound.
void CClock::HandleMsg(const void* event) {
    int type = ((const ClockMessage*)event)->type;
    switch (type) {
    case kClockReset: {
        const char* base = clockMessageStrings;
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->remainingTimeMs =
            AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs;
        AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->elapsedTimeMs = 0;
        countdownDisplayValue = kFloat_0_20943952_Clock;
        unsigned int color = kClockBlue;
        displayColor = color;
        running = 0;
        IActor_DispatchMessage(this, this, base, 8, color, (char*)&countdownDisplayValue, 0, 0);
        Sound_StopByAlias(g_soundManager, base + CLOCK_STRING_OFFSET(timeSound));
        break;
    }
    case 0:
        running = 1;
        if (GolfTask_NoOpStatus())
            return;
        break;
    case 1:
        running = 0;
        break;
    }
}
#pragma opt_common_subs reset

// Return the clock actor's type-information descriptor.
void* CClock::GetRTTI() {
    return &s_clockTypeInfo;
}

// Destroy the clock's native actor base and optionally free the clock object.
CClock::~CClock() {
    __dt__18NativeBase80201D18Fv(this, 0);
}

// Initialize the unit constant and register the clock actor's type and factories.
void CClock_RegisterType() {
    s_clockRegistrationTwoPi = kFloat_6_2831855;
    const char* base = clockMessageStrings;
    IntPair_Set(&s_clockTypeInfo, base + CLOCK_STRING_OFFSET(className), &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CClock_Create, kClockActorFactoryName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), kClockTaskFactoryName);
}

// Return the lazily constructed club parameter/resource table.
void* InstanceTable_Get() {
    return GetInstanceTable();
}

} // extern "C"

#pragma push
#pragma use_lmw_stmw on
// Free pooled hash nodes and bucket storage while holding the registry mutex.
inline TableHash::~TableHash() {
    {
        WInstanceLock wlock(&mutex);
        if (f08 != 0) {
            EA420Node* head;
            EA420Node* node;
            while ((node = (EA420Node*)f08) != 0) {
                head = EA420Remove(&mutex, node);
                f08 = (int)head;
                appFreeUni__FPv(node);
            }
        }
        if (buckets != 0) {
            appFreeUni__FPv(buckets);
            buckets = 0;
        }
    }
}
#pragma pop

// Unlink and free list nodes without deleting their separately managed payloads.
inline TableList::~TableList() {
    if (this != 0) {
        if (this != 0) {
            if (this != 0) {
                TreeLinks* sentinel = (TreeLinks*)&next;
                TreeLinks* node = (TreeLinks*)prev;
                if (node != sentinel) {
                    TreeLinks* sentinelNext = *(TreeLinks**)sentinel;
                    (*(TreeLinks**)node)->prev = sentinelNext->prev;
                    *(TreeLinks**)(sentinelNext->prev) = *(TreeLinks**)node;
                    while (node != sentinel) {
                        TreeLinks* cur = node;
                        node = node->prev;
                        appFreeUni__FPv(cur);
                        --f00;
                    }
                }
            }
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
// Release owned club records, recycle registry nodes and clear hash buckets before member destruction.
InstanceTableReal::~InstanceTableReal() {
    void* self = this;
    typedef EA420Node Node;
    char* base = (char*)self;
    void* data;
    {
        WInstanceLock lock(base + CLUB_FIELD_OFFSET(mutex));
        *(void**)(base + CLUB_FIELD_OFFSET(cursor)) = *(void**)(CLUB_FIELD_OFFSET(first) + base);
        {
            void* m0 = base + CLUB_FIELD_OFFSET(mutex);
            data = EA420Read(m0, base);
        }
    }
    if (0 != data) {
        void* m = base + CLUB_FIELD_OFFSET(mutex);
        while (data) {
            if (data)
                __dl__FPv(data);
            data = EA420Read(m, base);
        }
    }
    {
        WInstanceLock lock(base + CLUB_FIELD_OFFSET(mutex));
        if (*(Node**)(base + CLUB_FIELD_OFFSET(first)) != 0 && *(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)) != 0) {
            (*(Node**)(CLUB_FIELD_OFFSET(first) + base))->prev->next =
                (*(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)))->next;
            (*(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)))->next->prev =
                (*(Node**)(base + CLUB_FIELD_OFFSET(first)))->prev;
            (*(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)))->next = *(Node**)(base + CLUB_FIELD_OFFSET(first));
            (*(Node**)(base + CLUB_FIELD_OFFSET(first)))->prev = *(Node**)(base + CLUB_FIELD_OFFSET(freeNodes));
            *(void**)(base + CLUB_FIELD_OFFSET(first)) = 0;
        } else if (*(Node**)(base + CLUB_FIELD_OFFSET(first)) != 0) {
            Node* head;
            while ((head = *(Node**)(base + CLUB_FIELD_OFFSET(first))) != 0) {
                Node* next = EA420Remove(base + CLUB_FIELD_OFFSET(mutex), head);
                Node* current =
                    EA420Insert(base + CLUB_FIELD_OFFSET(mutex), *(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)),
                                *(Node**)(base + CLUB_FIELD_OFFSET(first)));
                *(Node**)(base + CLUB_FIELD_OFFSET(first)) = next;
                *(Node**)(base + CLUB_FIELD_OFFSET(freeNodes)) = current;
            }
        }
        int i = 0;
        void* zero = 0;
        int offset = 0;
        while (i < *(int*)(base + CLUB_FIELD_OFFSET(bucketCount))) {
            *(void**)((char*)*(void**)(base + CLUB_FIELD_OFFSET(buckets)) + offset) = zero;
            i += 1;
            offset = offset + 4;
        }
    }
}
#pragma pop

extern "C" {

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Release both four-resource arrays in each cache entry, delete entries and clear the loaded flag.
void InstanceTable_ClearCachedResources(void* self) {
    TreeLinks* iter;
    void* zero;
    TreeLinks* sentinel;
    TreeLinks* node;
    char* slot;
    TreeLinks* end;
    void* data;
    int i;
    sentinel = (TreeLinks*)(&((ClubSelectionStorage*)self)->resourceNext);
    zero = 0;
    iter = (TreeLinks*)((ClubSelectionStorage*)self)->resourcePrevious;
    while (iter != sentinel) {
        data = ((ClubCachedResourceNode*)iter)->resources;
        i = 0;
        slot = (char*)data;
        for (; i < 4; ++i) {
            if (g_resourceManager != 0 && ((ClubCachedResourceSet*)slot)->resourcesB[0] != 0) {
                ResourceManager_ReleaseObject((void*)g_resourceManager, ((ClubCachedResourceSet*)slot)->resourcesB[0],
                                              0);
                ((ClubCachedResourceSet*)slot)->resourcesB[0] = zero;
            }
            if (g_resourceManager != 0 && ((ClubCachedResourceSet*)slot)->resourcesA[0] != 0) {
                ResourceManager_ReleaseObject((void*)g_resourceManager, ((ClubCachedResourceSet*)slot)->resourcesA[0],
                                              0);
                ((ClubCachedResourceSet*)slot)->resourcesA[0] = zero;
            }
            slot += 4;
        }
        __dl__FPv(data);
        iter = iter->prev;
    }
    node = (TreeLinks*)((ClubSelectionStorage*)self)->resourcePrevious;
    end = (TreeLinks*)(&((ClubSelectionStorage*)self)->resourceNext);
    if (node != end) {
        TreeLinks* sentinelNext = end->next;
        node->next->prev = sentinelNext->prev;
        sentinelNext->prev->next = node->next;
        while (node != end) {
            TreeLinks* cur = node;
            node = node->prev;
            appFreeUni__FPv(cur);
            --((ClubSelectionStorage*)self)->resourceCount;
        }
    }
    ((ClubSelectionStorage*)self)->resourcesLoaded = 0;
}
#pragma pop

// Register wood, iron, wedge and putter defaults, reset selection state and select the first wood.
void CClub::Init() {
    void* node;
    float distanceYards = kFloat_230_Club;
    float launchDegrees = kFloat_10_GolfHud;
    float launchSpeed = kFloat_236_Club;
    float selectionScale = kFloat_1_61_Club;
    float selectionBias = kFloat_0_55_Club;
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings, kClubWood, distanceYards, launchDegrees, launchSpeed, selectionScale,
                  selectionBias);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(wood2), kClubWood, kFloat_210_ClubSelection,
                  kFloat_13_Club, kFloat_204_ClubSelection, kFloat_1_41_Club, kFloat_0_5_Camera);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(wood3), kClubWood, kFloat_190_Club, kFloat_16_Club,
                  kFloat_176_Club, kFloat_1_26_Club, kFloat_0_45_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron2), kClubIron, kFloat_180_ClubSelection,
                  kFloat_20_Club, kFloat_161_Club, kFloat_1_07_ClubSelection, kFloat_0_45_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron3), kClubIron, kFloat_170_Club, kFloat_24_Club,
                  kFloat_149_Club, kFloat_0_95_Club, kFloat_0_45_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron4), kClubIron, kFloat_160_Club,
                  kFloat_28_GolfDocHelpers, kFloat_139_Club, kFloat_0_83_ClubSelection, kFloat_0_45_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron5), kClubIron, kFloat_150_Club,
                  kFloat_32_ClubSelection, kFloat_131_Club, kFloat_0_73_Club, kFloat_0_45_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron6), kClubIron, kFloat_140_Club, kFloat_36_Club,
                  kFloat_124_Club, kFloat_0_67_Club, kFloat_0_41_ClubSelection);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron7), kClubIron, kFloat_130_Club, kFloat_40_Club,
                  kFloat_118_ClubSelection, kFloat_0_61_Club, kFloat_0_36_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron8), kClubIron, kFloat_120_Club, kFloat_44_Club,
                  kFloat_114_Club, kFloat_0_57_Club, kFloat_0_3_ClubSelection);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(iron9), kClubIron, kFloat_110_Club, kFloat_48_Club,
                  kFloat_110_Club, kFloat_0_53_ClubSelection, kFloat_0_25_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(pitchingWedge), kClubWedge, kFloat_100_Club,
                  kFloat_52_Club, kFloat_107_Club, kFloat_0_49_Club, kFloat_0_18_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(sandWedge), kClubWedge, kFloat_80_ClubSelection,
                  kFloat_56_Club, kFloat_93_ClubSelection, kFloat_0_42_Club, kFloat_0_17_ClubSelection);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter), kClubPutter, kFloat_20_Club,
                  kFloat_0_Club, kFloat_30_Club, kFloat_0_Club, kFloat_0_Club);
    ((CClub*)((HashManager*)this))
        ->AddClub((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(longPutter), kClubPutter, kFloat_10_GolfHud,
                  kFloat_0_Club, kFloat_21_Club, kFloat_0_Club, kFloat_0_Club);
    float zero = kFloat_0_Club;
    *(void**)this = 0;
    *(void**)&((ClubSelectionStorage*)this)->powerMode = 0;
    ((ClubSelectionStorage*)this)->distanceTier = kDistanceAtLeast58Yards;
    ((ClubSelectionStorage*)this)->opacity = zero;
    ((ClubSelectionStorage*)this)->slidePosition = zero;
    ((ClubSelectionStorage*)this)->animationState = kAnimationReady;
    ((ClubSelectionStorage*)this)->selectionFlag = 0;
    ((ClubSelectionStorage*)this)->previousClubIndex = 0;
    ((ClubSelectionStorage*)this)->powerBonus = zero;
    ((ClubSelectionStorage*)this)->resourcesLoaded = 0;
    char* key = (char*)&clubAndPlayerStrings;
    node = EA88CLookup(this, key);
    ((ClubSelectionStorage*)this)->selected = node;
    ((ClubSelectionStorage*)this)->clubIndex = ((ClubParameterView*)node)->clubIndex;
    *(void**)&((ClubSelectionStorage*)this)->inputState = 0;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Process club input/animation, track index/group changes, and update the gauge and optional camera transform.
// The club is read through its selection view in place; naming the view in a local changes the register allocation.
void CClub::Process(float delta) {
    void* manager = g_sharedDoc;
    void* state = (*(void**)&AsCSharedDoc(manager)->golfDoc);
    if (AsCGolfDoc(state)->tutorialStage >= 1) {
        ((CClub*)((ClubInputState*)this))->ClubProcess();
        ((CClub*)((ClubAnimationState*)this))->BarProcess(delta);
        unsigned char current = ((ClubSelectionStorage*)this)->clubIndex;
        if (((ClubSelectionStorage*)this)->previousClubIndex != current) {
            ((ClubSelectionStorage*)this)->changeFlags |= kSelectionChanged;
            void* first;
            {
                WInstanceLock guard(((ClubSelectionStorage*)this)->mutex);
                ((ClubSelectionStorage*)this)->cursor = ((ClubSelectionStorage*)this)->first;
                OSLockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                void* item = ((ClubSelectionStorage*)this)->cursor;
                if (item != 0) {
                    void* next = ((ItemEntry*)item)->next;
                    first = *(void**)item;
                    if (next == ((ClubSelectionStorage*)this)->first)
                        next = 0;
                    ((ClubSelectionStorage*)this)->cursor = next;
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                } else {
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                    first = 0;
                }
            }
            while (first != 0) {
                if (((ClubParameterView*)first)->clubIndex == current) {
                    current = ((ClubParameterView*)first)->clubGroup;
                    break;
                }
                OSLockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                void* item = ((ClubSelectionStorage*)this)->cursor;
                if (item != 0) {
                    void* next = ((ItemEntry*)item)->next;
                    first = *(void**)item;
                    if (next == ((ClubSelectionStorage*)this)->first)
                        next = 0;
                    ((ClubSelectionStorage*)this)->cursor = next;
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                } else {
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                    first = 0;
                }
            }
            if (first == 0)
                current = 0;

            unsigned char old = ((ClubSelectionStorage*)this)->previousClubIndex;
            {
                WInstanceLock guard(((ClubSelectionStorage*)this)->mutex);
                ((ClubSelectionStorage*)this)->cursor = ((ClubSelectionStorage*)this)->first;
                OSLockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                void* item = ((ClubSelectionStorage*)this)->cursor;
                if (item != 0) {
                    void* next = ((ItemEntry*)item)->next;
                    first = *(void**)item;
                    if (next == ((ClubSelectionStorage*)this)->first)
                        next = 0;
                    ((ClubSelectionStorage*)this)->cursor = next;
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                } else {
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                    first = 0;
                }
            }
            unsigned char value = 0;
            while (first != 0) {
                if (((ClubParameterView*)first)->clubIndex == old) {
                    value = ((ClubParameterView*)first)->clubGroup;
                    break;
                }
                OSLockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                void* item = ((ClubSelectionStorage*)this)->cursor;
                if (item != 0) {
                    void* next = ((ItemEntry*)item)->next;
                    first = *(void**)item;
                    if (next == ((ClubSelectionStorage*)this)->first)
                        next = 0;
                    ((ClubSelectionStorage*)this)->cursor = next;
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                } else {
                    OSUnlockMutex((OSMutex*)(((ClubSelectionStorage*)this)->mutex));
                    first = 0;
                }
            }
            if (current != value)
                ((ClubSelectionStorage*)this)->changeFlags |= kClubGroupChanged;
            else
                ((ClubSelectionStorage*)this)->changeFlags &= ~(unsigned int)kClubGroupChanged;

            void* mode = ((ClubSelectionStorage*)this)->selected;
            float gaugeCursorX =
                ((ClubParameterView*)mode)->clubGroup == kClubPutter ? kFloat_149_Club : kFloat_114_Club;
            state = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            AsCGolfDoc(state)->shotGaugeCursorX = gaugeCursorX;
            state = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            AsCGolfDoc(state)->roundedShotGaugeCursorX = AsCGolfDoc(state)->shotGaugeCursorX;
            state = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (((*(unsigned int*)&AsCGolfDoc(state)->gameStateFlags) & 0x200) != 0 &&
                ((ClubSelectionStorage*)this)->clubIndex < ((ClubSelectionStorage*)this)->previousClubIndex &&
                ((ClubSelectionStorage*)this)->distanceTier == kDistanceAtLeast58Yards) {
                char text[0x50];
                CGolfDoc::BuildModeCameraTransform(text, state, 1);
                Matrix_CopyAffine(s_cameraTransform, text);
            }
            ((ClubSelectionStorage*)this)->previousClubIndex = ((ClubSelectionStorage*)this)->clubIndex;
        } else {
            ((ClubSelectionStorage*)this)->changeFlags &= ~(unsigned int)kSelectionChangeMask;
        }
    }
}
#pragma pop

// Gate previous/next club input by menus, camera state and party-item locks, then update selection and sounds.
void CClub::ClubProcess() {
    ClubInputState* self = (ClubInputState*)this;
    if (!CItemWindow::IsClosed())
        return;
    if (Pointer_IsActionActive(g_inputPointer, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(leftAlt), 1) ||
        Pointer_IsActionActive(g_inputPointer, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(rightAlt), 1))
        return;
    if (self->inputState != kAnimationReady)
        return;
    int flag = 0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (char*)InputManager_Dtor, s_inputManagerDtorRegistration_Club);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    if (!Input_IsActionButtonDown(GetInputManagerSlot(), kActionCameraModifier, -1)) {
        unsigned char* p = (unsigned char*)g_sharedDoc;
        int s = AsCGolfDoc((void*)AsCSharedDoc(p)->golfDoc)->cameraMode;
        if (s && s != 4)
            flag = 1;
    }
    int q = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    int response;
    EAMessage args(0x15, &response);
    EAReceiver* obj =
        (EAReceiver*)TaskActorTable_FindByName(q, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(player));
    if (obj)
        obj->send(&args);
    if (response == 3)
        return;
    unsigned char* root = (unsigned char*)g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(root)->partyGameMode) == 3) {
        int count = (int)PartyItemSlots_GetInstance();
        for (int i = 0; i < AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->playerCount; ++i)
            if (PartyItemSlots_GetItemType(count, i) == 11) {
                if (!GetInputManagerSlot()) {
                    if (!s_inputManagerInitialized) {
                        InputManager_Init(s_gameInputManager);
                        __register_global_object(s_gameInputManager, (char*)InputManager_Dtor,
                                                 s_inputManagerDtorRegistration_Club);
                        s_inputManagerInitialized = 1;
                    }
                    GetInputManagerSlot() = s_gameInputManager;
                }
                if (!Input_IsActionPressed(GetInputManagerSlot(), kActionNextClub, -1)) {
                    if (!GetInputManagerSlot()) {
                        if (!s_inputManagerInitialized) {
                            InputManager_Init(s_gameInputManager);
                            __register_global_object(s_gameInputManager, (char*)InputManager_Dtor,
                                                     s_inputManagerDtorRegistration_Club);
                            s_inputManagerInitialized = 1;
                        }
                        GetInputManagerSlot() = s_gameInputManager;
                    }
                    if (!Input_IsActionPressed(GetInputManagerSlot(), kActionPreviousClub, -1))
                        return;
                }
                if (flag)
                    return;
                Sound_PlayByName(g_soundManager, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(unplayableSound), 0,
                                 0, 0, 0, kFloat_0_5_Camera, kFloat_200_Club, kFloat_1_golfParameters);
                NoticeQueue_ShowById(g_helpMsg, 3, 0);
                return;
            }
    }
    int size;
    {
        WInstanceLock lock(self->mutex);
        EANode* first = self->first;
        size = 0;
        EANode* node = first;
        while (node) {
            node = first != node->next ? node->next : 0;
            ++size;
        }
    }
    if (self->clubIndex < size - 1) {
        EAInput();
        if (Input_IsActionPressed(GetInputManagerSlot(), kActionNextClub, -1) && !flag) {
            int q2 = g_taskManager ? Object_GetChild(g_taskManager) : 0;
            int response2;
            EAMessage args2(0x36, &response2);
            EAReceiver* obj2 =
                (EAReceiver*)TaskActorTable_FindByName(q2, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(screen));
            if (obj2)
                obj2->send(&args2);
            if (self->powerMode || response2) {
                unsigned char position = (unsigned char)(self->clubIndex + 1);
                SELECT_CLUB_AT_INDEX(value, position);
                self->selected = value;
                if (!value || ((ClubParameterView*)value)->clubGroup == kClubPutter) {
                    unsigned char position = self->clubIndex;
                    SELECT_CLUB_AT_INDEX(old, position);
                    self->selected = old;
                    Sound_PlayByName(g_soundManager, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(unplayableSound),
                                     0, 0, 0, 0, kFloat_0_5_Camera, kFloat_200_Club, kFloat_1_golfParameters);
                    if (g_helpMsg)
                        NoticeQueue_ShowById(g_helpMsg, 1, 0);
                } else
                    ++self->clubIndex;
            } else {
                unsigned char position = (unsigned char)(self->clubIndex + 1);
                self->clubIndex = position;
                SELECT_CLUB_AT_INDEX(value, position);
                self->selected = value;
            }
            Sound_PlayByName(g_soundManager, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(changeSound), 0, 0, 0, 0,
                             kFloat_0_5_Camera, kFloat_200_Club, kFloat_1_golfParameters);
        }
    }
    EAInput();
    if (!Input_IsActionPressed(GetInputManagerSlot(), kActionPreviousClub, -1))
        return;
    if (flag)
        return;
    if (self->clubIndex > self->minimumClubIndex) {
        unsigned char position = (unsigned char)(self->clubIndex - 1);
        self->clubIndex = position;
        SELECT_CLUB_AT_INDEX(value, position);
        self->selected = value;
        Sound_PlayByName(g_soundManager, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(changeSound), 0, 0, 0, 0,
                         kFloat_0_5_Camera, kFloat_200_Club, kFloat_1_golfParameters);
    } else if (g_helpMsg && self->minimumClubIndex) {
        Sound_PlayByName(g_soundManager, (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(unplayableSound), 0, 0, 0, 0,
                         kFloat_0_5_Camera, kFloat_200_Club, kFloat_1_golfParameters);
        NoticeQueue_ShowById(g_helpMsg, 2, 0);
    }
}

// Advance the delay, sliding and fade-in stages, restarting them when the selected club changes.
void CClub::BarProcess(float delta) {
    ClubAnimationState* self = (ClubAnimationState*)this;
    switch (self->animationState) {
    case kAnimationDelay:
        if (self->flags & kSelectionChanged)
            self->fadeSeconds = kFloat_0_Club;
        self->fadeSeconds += delta;
        if (self->fadeSeconds > kFloat_0_7_Club)
            self->animationState = kAnimationSlideOut;
        break;
    case kAnimationSlideOut:
        if (self->flags & kSelectionChanged) {
            self->animationState = kAnimationFadeIn;
            self->opacity = kFloat_0_Club;
            self->slidePosition = kFloat_456_Club;
        } else {
            self->slidePosition = self->slidePosition - kFloat_1000_Club * delta;
            if (self->slidePosition < kFloat_0_Club) {
                self->slidePosition = kFloat_0_Club;
                self->animationState = kAnimationReady;
                GolfTask_NoOpStatus();
            }
        }
        break;
    case kAnimationFadeIn:
        self->opacity = self->opacity + kFloat_765_ClubSelection * delta;
        if (self->opacity > kFloat_255_Club) {
            self->opacity = kFloat_255_Club;
            self->fadeSeconds = kFloat_0_Club;
            self->animationState = kAnimationDelay;
        }
        break;
    case kAnimationReady:
        if (self->flags & kSelectionChanged) {
            unsigned char* p = (unsigned char*)GolfBall__Fv();
            if (((CGolfBall*)p)->m_groundType == kGroundGreen)
                GolfTask_NoOpStatus();
            else {
                self->animationState = kAnimationFadeIn;
                self->opacity = kFloat_0_Club;
                self->slidePosition = kFloat_456_Club;
            }
        }
        break;
    }
}

// Update or allocate a named club record, assign its list index and look up its parameter-kind bias.
// groupOrListCount starts as kClubWood/Iron/Wedge/Putter; after storing the group,
// its storage is reused only to count the registry list. Records belong to self.
void CClub::AddClub(const char* name, unsigned int groupOrListCount, float distanceYards, float launchDegrees,
                    float launchSpeed, float selectionScale, float selectionBias) {
    HashManager* self = (HashManager*)this;
    Entry* entry;

    entry = HashFind(self, name);
    if (entry) {
        entry->distanceYards = distanceYards;
        entry->launchAngle = kGolfDegreesToRadians * launchDegrees;
        entry->clubGroup = groupOrListCount;
        entry->launchSpeed = launchSpeed;
        entry->selectionScale = selectionScale;
        entry->selectionBias = selectionBias;
        return;
    }
    entry = new Entry;
    entry->u.tail.scale = kFloat_1_golfParameters;
    entry->distanceYards = distanceYards;
    entry->launchAngle = kGolfDegreesToRadians * launchDegrees;
    entry->clubGroup = groupOrListCount;
    entry->launchSpeed = launchSpeed;
    entry->selectionScale = selectionScale;
    entry->selectionBias = selectionBias;
    {
        void* lk = &self->lock;
        OSLockMutex((OSMutex*)lk);
        HashNode* start = self->first;
        HashNode* cur = start;
        // Registration has consumed the group; count the existing list entries.
        groupOrListCount = 0;
        while (cur) {
            HashNode* next = cur->prev;
            if (start != next)
                cur = next;
            else
                cur = 0;
            groupOrListCount++;
        }
        OSUnlockMutex((OSMutex*)lk);
        entry->listIndex = (unsigned char)groupOrListCount;
    }
    strcpy(entry->u.name, name);
    HashInsert(self, entry);
    unsigned char i = 0;
    while (i < kClubNameCount) {
        if (!strcmp(entry->u.name, s_clubParameterKindNames[i])) {
            entry->kind = i;
            break;
        }
        ++i;
    }
    entry->u.tail.preferenceBias = s_clubParameterPreferenceBiases[entry->kind];
}

// Choose a club for the current hole's target position.
void CClub::SelectBestClubForHole() {
    struct V3EBE70 {
        unsigned int a;
        unsigned int b;
        unsigned int c;
    };
    unsigned char* mgr = (*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    V3EBE70 data =
        *(V3EBE70*)((*(unsigned char**)&AsCGolfDoc(mgr)->holeRecords) + (AsCGolfDoc(mgr)->currentHoleIndex << 5) + 0xC);
    SelectBestClub(&data);
}

#pragma push
#pragma opt_common_subs off
// Subtract the selector's landing penalty while leaving unlisted terrains unchanged.
// Score is updated once per terrain sample; outside-course penalties are applied by the caller.
static inline void PenalizeClubLandingTerrain(int terrainCode, float& candidateScore) {
    switch (terrainCode) {
    case kGroundBunker:
        candidateScore -= kFloat_200_Club;
        break;
    case kPenalty100Ground4:
    case kPenalty100Ground6:
    case kPenalty100Ground7:
    case kPenalty100Ground10:
        candidateScore -= kFloat_100_Club;
        break;
    case kGroundWater:
        candidateScore -= kFloat_1000_Club;
        break;
    case kUnpenalizedGround5:
    case kUnpenalizedGround8:
        break;
    }
}

// Choose a club from target distance, player stats, lie slope and sampled terrain; apply green/bunker/tee rules.
// targetPosition is a borrowed world-space point. Horizontal distance / 3.2 gives
// the yard-like distance used by club parameters; height does not affect selection.
// Policy order: distance/control setup -> green putter lookup OR off-green terrain
// scoring -> short-approach override -> bunker override -> opening-tee override.
// Later overrides win. Registry initialization must provide all named/default clubs;
// a missing record is not checked before dereference. Selection keeps borrowed records.
// Select a borrowed club record using target range, sampled terrain and ordered overrides.
// The state is read through ClubSelectionState in place; naming the view in a local changes the register allocation.
void CClub::SelectBestClub(void* targetPosition) {
    // Phase 1: choose the <10, <15, <28, <58, or >=58 distance tier and control window.
    Vec3 v;
    __mi__FRC7WVectorRC7WVector(&v, GolfBall__Fv(), targetPosition);
    v.y = kFloat_0_Club;
    float targetDistanceYards = Magnitude__7WVectorCFv(&v) / kFloat_3_2_Club;
    ((ClubSelectionState*)this)->selectionPolicyValue = 0;
    if (targetDistanceYards < kFloat_10_GolfHud) {
        ((ClubSelectionState*)this)->distanceTier = kDistanceBelow10Yards;
    } else if (targetDistanceYards < kFloat_15_Club) {
        ((ClubSelectionState*)this)->distanceTier = kDistance10To15Yards;
    } else if (targetDistanceYards < kFloat_28_GolfDocHelpers) {
        ((ClubSelectionState*)this)->distanceTier = kDistance15To28Yards;
    } else if (targetDistanceYards < kFloat_58_Club) {
        ((ClubSelectionState*)this)->distanceTier = kDistance28To58Yards;
    } else {
        ((ClubSelectionState*)this)->distanceTier = kDistanceAtLeast58Yards;
    }

    PlayerControlStatsPrefix* inst = (PlayerControlStatsPrefix*)GolfPlayer_GetEntryByIndex(
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);

    ((ClubSelectionState*)this)->controlValue =
        (float)(int)inst->control + s_clubGroundTypeScoreOffsets[((CGolfBall*)GolfBall__Fv())->m_groundType];
    float controlLimit = (kFloat_10_GolfHud + ((ClubSelectionState*)this)->controlValue);
    ((ClubSelectionState*)this)->controlLimit = controlLimit > kFloat_35_GolfHud ? kFloat_35_GolfHud : controlLimit;

    if (((CGolfBall*)GolfBall__Fv())->m_groundType == kGroundGreen) {
        // Phase 2a: fixed putter policy. Each FIND call searches the locked name
        // registry (not a direct array lookup); the last argument is a unique label.
        // longPutt and the chosen named putter are independent parts of this policy.
        ClubParameterPrefix* found;
        switch (((ClubSelectionState*)this)->distanceTier) {
        case kDistanceBelow10Yards:
            ((ClubSelectionState*)this)->longPutt = 0;
            FIND_CLUB_BY_NAME(((ClubSelectionState*)this),
                              (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(longPutter), found, 0);
            ((ClubSelectionState*)this)->selected = found;
            break;
        case kDistance10To15Yards:
            ((ClubSelectionState*)this)->longPutt = 0;
            FIND_CLUB_BY_NAME(((ClubSelectionState*)this),
                              (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter), found, 1);
            ((ClubSelectionState*)this)->selected = found;
            break;
        case kDistance15To28Yards:
            ((ClubSelectionState*)this)->longPutt = 1;
            FIND_CLUB_BY_NAME(((ClubSelectionState*)this),
                              (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(longPutter), found, 2);
            ((ClubSelectionState*)this)->selected = found;
            break;
        case kDistance28To58Yards:
        case kDistanceAtLeast58Yards:
            ((ClubSelectionState*)this)->longPutt = 1;
            FIND_CLUB_BY_NAME(((ClubSelectionState*)this),
                              (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter), found, 3);
            ((ClubSelectionState*)this)->selected = found;
            break;
        }
        ((ClubSelectionState*)this)->clubIndex = ((ClubSelectionState*)this)->selected->clubIndex;
        // Forced selection value, not thirteen measured slope rejections.
        ((ClubSelectionState*)this)->selectionPolicyValue = kPutterIndex;
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeCursorX = kFloat_149_Club;
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->roundedShotGaugeCursorX =
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeCursorX;
    } else {
        float bestScore = kFloat_neg10000_ClubSelection;
        // Phase 2b: aim horizontally along the current aim path, not necessarily
        // toward targetPosition. All terrain samples use that same normalized ray.
        unsigned char lvl = 1;

        ((ClubSelectionState*)this)->longPutt = lvl;
        void* cm = GolfBall__Fv();
        int p = ((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(cm, 0);
        CGolfDoc* inst2 = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* cm2 = GolfBall__Fv();
        Vec3 v3;
        Vec3 v2;
        GolfAim_FindTargetAlongPath(&v2, inst2->aimPath, (char*)&clubAndPlayerStrings);
        __mi__FRC7WVectorRC7WVector(&v3, &v2, cm2);
        v3.y = kFloat_0_Club;
        Normalize__7WVectorFv(&v3);
        unsigned int hazardAhead = 0;
        // Extra terrain policy applies only below rank tier 6 and course flag nibble 3.
        // First probe the default named club at 80%, 90%, 100% of its resolved range.
        // Type 9 or an outside-playable-region sample enables candidate penalties.
        if (PlayerRank_FindTierForValue(
                ((SharedRankSlots*)g_sharedDoc)
                    ->players[AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer]
                    .rank) < 6) {
            void* q = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
            lvl = !q ? lvl : ((((IFF_STRUCT::sCourse*)q)->courseFlags >> 4) & 0xf);
            if (3 > lvl) {
                float clubDistance =
                    GolfParameter_ResolveFloat((ClubSelectionState*)this, (char*)&clubAndPlayerStrings);

                for (float t = kFloat_0_8_Club; t < kFloat_1_01_ClubSampling; t += kFloat_0_1_ClubTrajectory) {
                    int ok = 1;
                    Vec3i c;
                    Vec3 samplePoint;
                    Vec3 offset;
                    float range = kFloat_3_2_Club * clubDistance;
                    __ml__FfRC7WVector(&offset, &v3, t * range);
                    __pl__FRC7WVectorRC7WVector(&samplePoint, GolfBall__Fv(), &offset);
                    c = *(Vec3i*)&samplePoint;
                    if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), (Vec3*)&c) != kGroundWater) {
                        Vec3 d;
                        CGolfDoc* in = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
                        Vec3 e;
                        __ml__FfRC7WVector(&d, &v3, t * ((kFloat_3_2_Club * clubDistance)));
                        __pl__FRC7WVectorRC7WVector(&e, GolfBall__Fv(), &d);
                        if (CourseData_IsPointOutsidePlayableRegion(in->courseData, &e) == 0) {
                            ok = 0;
                        }
                    }
                    if (ok) {
                        hazardAhead = 1;
                        break;
                    }
                }
            }
        }

        ClubParameterPrefix* item;
        {
            WInstanceLock l1(((ClubSelectionState*)this)->mutex);
            ((ClubSelectionState*)this)->current = ((ClubSelectionState*)this)->first;
            ADVANCE((ClubSelectionState*)this, item);
        }
        while (item) {
            // Phase 3: score non-putter indices 0..12. A launch below the current
            // triangle slope + 0.06981317 radians (4 degrees) is rejected outright.
            if (item->clubIndex < kPutterIndex) {
                if (item->launchAngle <
                    ((CQuadTree*)CQuadTree_GetInstance())->GetGroundSlope(p) + kFloat_0_06981317_GolfDocHelpers) {
                    ((ClubSelectionState*)this)->selectionPolicyValue++;
                } else {
                    float clubDistance = GolfParameter_ResolveFloat((ClubSelectionState*)this, &item->selection.name);
                    float distanceErrorYards = clubDistance - targetDistanceYards;
                    distanceErrorYards = distanceErrorYards > kFloat_0_Club ? distanceErrorYards : -distanceErrorYards;
                    float candidateScore = FP_FMADDS(kFloat_0_5_Camera, item->selection.bias,
                                                     (kFloat_neg1_ClubSelection * distanceErrorYards));
                    // Score = half preference bias - absolute distance error;
                    // add 10 when range exceeds the target. Higher scores win.
                    if (clubDistance > targetDistanceYards) {
                        candidateScore = candidateScore + kFloat_10_GolfHud;
                    }
                    if (hazardAhead) {
                        // Sum penalties at each 80/90/100% landing sample: outside
                        // playable region/type 9: -1000; type 3: -200; types 4/6/7/10:
                        // -100; types 5/8: unchanged. Unlisted types also add nothing.
                        // Numeric terrain identities are retained where unresolved.

                        for (float t = kFloat_0_8_Club; t < kFloat_1_01_ClubSampling; t += kFloat_0_1_ClubTrajectory) {
                            Vec3 samplePoint;
                            Vec3 offset;
                            CGolfDoc* in = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
                            __ml__FfRC7WVector(&offset, &v3, t * (kFloat_3_2_Club * clubDistance));
                            __pl__FRC7WVectorRC7WVector(&samplePoint, GolfBall__Fv(), &offset);
                            if (CourseData_IsPointOutsidePlayableRegion(in->courseData, &samplePoint) != 0) {
                                candidateScore -= kFloat_1000_Club;
                            } else {
                                Vec3i c2;
                                Vec3 e2;
                                Vec3 d2;
                                __ml__FfRC7WVector(&d2, &v3, t * (kFloat_3_2_Club * clubDistance));
                                __pl__FRC7WVectorRC7WVector(&e2, GolfBall__Fv(), &d2);
                                c2 = *(Vec3i*)&e2;

                                PenalizeClubLandingTerrain(
                                    GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), (Vec3*)&c2),
                                    candidateScore);
                            }
                        }
                    }
                    candidateScore = candidateScore * item->selection.weight;
                    // Weight scales the whole score including penalties. Strict >
                    // preserves the first registered club when scores tie.
                    if (candidateScore > bestScore) {
                        bestScore = candidateScore;
                        ((ClubSelectionState*)this)->selected = item;
                        ((ClubSelectionState*)this)->clubIndex = item->clubIndex;
                    }
                }
            }
            ADVANCE((ClubSelectionState*)this, item);
        }

        if (((ClubSelectionState*)this)->distanceTier < kDistanceAtLeast58Yards) {
            // Phase 4: short approaches replace the scored result with list entry
            // 11 (zero-based). The selection-state byte is floored at 7, not recounted.
            ((ClubSelectionState*)this)->selectionPolicyValue = ((ClubSelectionState*)this)->selectionPolicyValue > 7
                                                                    ? ((ClubSelectionState*)this)->selectionPolicyValue
                                                                    : 7;
            {
                WInstanceLock l2(((ClubSelectionState*)this)->mutex);
                ((ClubSelectionState*)this)->current = ((ClubSelectionState*)this)->first;
                ADVANCE((ClubSelectionState*)this, item);
            }
            unsigned int i = 0;
            do {
                i++;
                ADVANCE((ClubSelectionState*)this, item);
            } while (kFirstWedgeIndex > i);
            ((ClubSelectionState*)this)->selected = item;
            ((ClubSelectionState*)this)->clubIndex = item->clubIndex;
        }
        if (((CGolfBall*)GolfBall__Fv())->m_groundType == kGroundBunker) {
            // Bunker policy replaces even the short-approach choice with list entry 12.
            ClubParameterPrefix* item;
            {
                WInstanceLock l3(((ClubSelectionState*)this)->mutex);
                ((ClubSelectionState*)this)->current = ((ClubSelectionState*)this)->first;
                ADVANCE((ClubSelectionState*)this, item);
            }
            unsigned int i = 0;
            do {
                i++;
                ADVANCE((ClubSelectionState*)this, item);
            } while (i < kSandWedgeIndex);
            ((ClubSelectionState*)this)->selected = item;
            ((ClubSelectionState*)this)->clubIndex = item->clubIndex;
        }
    }

    if (((CGolfBall*)GolfBall__Fv())->m_groundType == kGroundTee) {
        // Phase 5: on the opening tee stroke, cap indices above 2 to named wood3.
        GolfDocSnapshot8013* inst3 = (GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);

        if (inst3->holeStrokeCounts[AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex] <= 1) {
            if (2 < ((ClubSelectionState*)this)->clubIndex) {
                ClubParameterPrefix* found3;
                FIND_CLUB_BY_NAME(((ClubSelectionState*)this),
                                  (CLUB_STRING_OFFSET(wood3) + (char*)&clubAndPlayerStrings), found3, 4);
                ((ClubSelectionState*)this)->selected = found3;
                ((ClubSelectionState*)this)->clubIndex = found3->clubIndex;
            }
        }
    }

    ((ClubSelectionState*)this)->previousClubIndex = ((ClubSelectionState*)this)->clubIndex;
}
#pragma pop

#pragma push
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Choose a CPU club from distance, lie and player stats, including green/tee and power-flag overrides.
// Shares the human selector's horizontal distance tiers and slope/score rules,
// but does not run the human terrain-sampling penalty pass. Inspect the final
// power/tee overrides before changing its otherwise similar candidate loop.
void ClubSelection_SelectForCpuTarget(ClubSelectionState* self, void* targetPosition) {
    int p;
    ClubSelectionState* s;
    PlayerControlStatsPrefix* inst;
    float count;
    unsigned char mode;
    Vec3 v;
    s = self;
    __mi__FRC7WVectorRC7WVector(&v, GolfBall__Fv(), targetPosition);
    v.y = kFloat_0_Club;
    float targetDistanceYards = Magnitude__7WVectorCFv(&v) / kFloat_3_2_Club;
    s->selectionPolicyValue = 0;
    if (targetDistanceYards < kFloat_10_GolfHud) {
        s->distanceTier = kDistanceBelow10Yards;
    } else if (targetDistanceYards < kFloat_15_Club) {
        s->distanceTier = kDistance10To15Yards;
    } else if (targetDistanceYards < kFloat_28_GolfDocHelpers) {
        s->distanceTier = kDistance15To28Yards;
    } else if (targetDistanceYards < kFloat_58_Club) {
        s->distanceTier = kDistance28To58Yards;
    } else {
        s->distanceTier = kDistanceAtLeast58Yards;
    }

    inst = (PlayerControlStatsPrefix*)GolfPlayer_GetEntryByIndex(
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
    mode = ((CGolfBall*)GolfBall__Fv())->m_groundType;
    count = (float)(int)inst->control;
    s->controlValue = count + s_clubGroundTypeScoreOffsets[mode];
    float controlLimit = kFloat_10_GolfHud + s->controlValue;
    s->controlLimit = controlLimit > kFloat_35_GolfHud ? kFloat_35_GolfHud : controlLimit;

    switch (((CGolfBall*)GolfBall__Fv())->m_groundType) {
    case kGroundGreen: {
        ClubParameterPrefix* found;
        switch (s->distanceTier) {
        case kDistanceBelow10Yards:
            s->longPutt = 0;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(longPutter), found, 0);
            s->selected = found;
            break;
        case kDistance10To15Yards:
            s->longPutt = 0;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter), found, 1);
            s->selected = found;
            break;
        case kDistance15To28Yards:
            s->longPutt = 1;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(longPutter), found, 2);
            s->selected = found;
            break;
        case kDistance28To58Yards:
        case kDistanceAtLeast58Yards:
            s->longPutt = 1;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter), found, 3);
            s->selected = found;
            break;
        }
        s->clubIndex = s->selected->clubIndex;
        s->selectionPolicyValue = kPutterIndex;
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeCursorX = kFloat_149_Club;
        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->roundedShotGaugeCursorX =
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->shotGaugeCursorX;
    } break;
    case kGroundTee: {
        GolfDocSnapshot8013* inst3 = (GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
            AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
        unsigned char off = AsCGolfDoc(AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
        if (inst3->holeStrokeCounts[off] <= 1 && s->clubIndex > 2) {
            ClubParameterPrefix* found3;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(wood3), found3, 4);
            s->selected = found3;
            s->clubIndex = found3->clubIndex;
        }
    } break;
    default: {
        s->longPutt = 1;
        if (*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_powerFlags & 0x20) {
            ClubParameterPrefix* found4;
            FIND_CPU_CLUB_BY_NAME((char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(wood3), found4, 5);
            s->selected = found4;
            s->clubIndex = found4->clubIndex;
        } else {
            float bestScore = kFloat_neg10000_ClubSelection;
            void* cm = GolfBall__Fv();
            p = ((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(cm, 0);
            CGolfDoc* inst2 = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            void* cm2 = GolfBall__Fv();
            Vec3 v3, v2;
            GolfAim_FindTargetAlongPath(&v2, inst2->aimPath, (char*)&clubAndPlayerStrings);
            __mi__FRC7WVectorRC7WVector(&v3, &v2, cm2);
            v3.y = kFloat_0_Club;
            Normalize__7WVectorFv(&v3);

            ClubParameterPrefix* item;
            {
                WInstanceLock l1(s->mutex);
                s->current = s->first;
                ADVANCE(s, item);
            }
            while (item) {
                if (item->clubIndex < kPutterIndex) {
                    if (item->launchAngle <
                        ((CQuadTree*)CQuadTree_GetInstance())->GetGroundSlope(p) + kFloat_0_06981317_GolfDocHelpers) {
                        s->selectionPolicyValue++;
                    } else {
                        float clubDistance = GolfParameter_ResolveFloat(s, &item->selection.name);
                        float distanceErrorYards = clubDistance - targetDistanceYards;
                        distanceErrorYards =
                            distanceErrorYards > kFloat_0_Club ? distanceErrorYards : -distanceErrorYards;
                        float candidateScore =
                            kFloat_0_5_Camera * item->selection.bias + kFloat_neg1_ClubSelection * distanceErrorYards;
                        if (clubDistance > targetDistanceYards) {
                            candidateScore += kFloat_10_GolfHud;
                        }
                        candidateScore *= item->selection.weight;
                        if (candidateScore > bestScore) {
                            s->selected = item;
                            bestScore = candidateScore;
                            s->clubIndex = item->clubIndex;
                        }
                    }
                }
                ADVANCE(s, item);
            }
        }
    } break;
    }

    s->previousClubIndex = s->clubIndex;
}

#pragma pop

// Recompute the current player's control value plus lie adjustment and its capped limit.
void ShotParameters_UpdateControlWindow(ProgressState* self) {
    void* root = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char index = AsCGolfDoc(root)->currentPlayer;
    void* state = GolfPlayer_GetEntryByIndex(root, index);
    void* game = GolfBall__Fv();
    int value = ((PlayerShotStats*)state)->control;
    unsigned char slot = ((CGolfBall*)game)->m_groundType;
    float offset = s_clubGroundTypeScoreOffsets[slot];
    float progress = (float)value + offset;
    self->progress = progress;
    float limit = kFloat_10_GolfHud + progress;
    limit = (limit > kFloat_35_GolfHud) ? kFloat_35_GolfHud : limit;
    self->limit = limit;
}

// Compute the good-impact half-width from control, matching inventory items and party width modifiers.
float ShotParameters_GetGoodImpactHalfWidth(void* self) {
    if (((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->clubGroup == kClubPutter)
        return kFloat_0_Club;
    void* progress = PartyItemSlots_GetInstance();
    InstanceTableReal* table = GetInstanceTable();
    float rate = ((ClubSelectionStorage*)table)->powerMode ? kFloat_0_6_Club : kFloat_1_golfParameters;
    float converted = (float)ceil(((ClubSelectionStorage*)self)->controlValue * rate);
    float value = converted < kFloat_2_golfParameters ? kFloat_2_golfParameters : converted;
    void* entry;
    int id;
    {
        void* m = g_sharedDoc;
        void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
        entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                           AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
        id = ItemInventory_GetItemIdAt(sub, 2);
    }
    // 0x800ED6BC: skip the multiplier only if all four items differ.
    do {
        if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
            {
                void* m = g_sharedDoc;
                void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                   AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                id = ItemInventory_GetItemIdAt(sub, 7);
            }
            if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
                {
                    void* m = g_sharedDoc;
                    void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                    entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                       AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                    id = ItemInventory_GetItemIdAt(sub, 8);
                }
                if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
                    {
                        void* m = g_sharedDoc;
                        void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                        entry =
                            GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                       AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                        id = ItemInventory_GetItemIdAt(sub, 9);
                    }
                    if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id)
                        break;
                }
            }
        }
        value *= kFloat_2_golfParameters;
    } while (0);

    ;
    float numerator = value * ((ShotAdjustmentState*)progress)->impactWidthBonus;
    FP_FMADDS_ACCUM(value, numerator, kFloat_0_5_Camera);
    value /= ((ShotAdjustmentState*)progress)->impactWidthDivisor;
    return value;
}

// Compute the poor-impact half-width from capped control, matching inventory items and party modifiers.
float ShotParameters_GetPoorImpactHalfWidth(void* self) {
    if (((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->clubGroup == kClubPutter)
        return kFloat_0_Club;
    void* progress = PartyItemSlots_GetInstance();
    InstanceTableReal* table = GetInstanceTable();
    float rate = ((ClubSelectionStorage*)table)->powerMode ? kFloat_0_6_Club : kFloat_1_golfParameters;
    float converted = (float)ceil(((ClubSelectionStorage*)self)->controlValue * rate);
    float value = converted < kFloat_2_golfParameters ? kFloat_2_golfParameters : converted;
    float cap = kFloat_10_GolfHud + value;
    float factor = cap > kFloat_35_GolfHud ? kFloat_35_GolfHud : cap;
    void* entry;
    int id;
    {
        void* m = g_sharedDoc;
        void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
        entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                           AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
        id = ItemInventory_GetItemIdAt(sub, 2);
    }
    // 0x800ED944: skip the multiplier only if all four items differ.
    do {
        if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
            {
                void* m = g_sharedDoc;
                void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                   AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                id = ItemInventory_GetItemIdAt(sub, 7);
            }
            if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
                {
                    void* m = g_sharedDoc;
                    void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                    entry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                       AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                    id = ItemInventory_GetItemIdAt(sub, 8);
                }
                if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id) {
                    {
                        void* m = g_sharedDoc;
                        void* sub = (char*)&AsCSharedDoc(m)->itemInventory;
                        entry =
                            GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(m)->golfDoc),
                                                       AsCGolfDoc((*(void**)&AsCSharedDoc(m)->golfDoc))->currentPlayer);
                        id = ItemInventory_GetItemIdAt(sub, 9);
                    }
                    if ((unsigned int)((GolfDocSnapshot8013*)entry)->itemId != (unsigned int)id)
                        break;
                }
            }
        }
        factor *= kFloat_2_golfParameters;
    } while (0);

    ;
    float numerator = factor * ((ShotAdjustmentState*)progress)->impactWidthBonus;
    FP_FMADDS_ACCUM(factor, numerator, kFloat_0_5_Camera);
    factor /= ((ShotAdjustmentState*)progress)->impactWidthDivisor;
    return factor;
}

// Resolve named or selected club distance, then apply club-group, power-mode, equipment and party modifiers.
float GolfParameter_ResolveFloat(void* self, void* keyArg) {
    const char* key = (const char*)keyArg;
    void* node;
    if (key != 0) {
        node = ((TargetLookup*)self)->find(key);
    } else {
        node = ((ClubSelectionStorage*)self)->selected;
    }

    float result = *(float*)node;
    unsigned char type = ((ClubParameterView*)node)->clubGroup;
    switch (type) {
    case 0: {
        float offset;
        void* manager = g_sharedDoc;
        void* root = (*(void**)&AsCSharedDoc(manager)->golfDoc);
        void* state = GolfPlayer_GetEntryByIndex(root, AsCGolfDoc(root)->currentPlayer);
        int value = ((PlayerShotStats*)state)->power;
        Arr3 table = s_golfParameterModeOffsets;
        offset = table.v[*(unsigned int*)&((ClubSelectionStorage*)self)->powerMode];
        result = FP_FMADDS(kFloat_2_golfParameters, (float)value, (*(float*)node + offset));
        break;
    }
    case 1: {
        Arr3 table = s_golfParameterModeOffsets;
        result += table.v[*(unsigned int*)&((ClubSelectionStorage*)self)->powerMode];
        break;
    }
    case 2:
        switch (((ClubSelectionStorage*)self)->distanceTier) {
        case 0:
        case 1:
        case 2: {
            unsigned int index = *(unsigned int*)&((ClubSelectionStorage*)self)->powerMode;
            result = kFloat_30_Club + (index ? kFloat_30_Club : kFloat_0_Club);
            break;
        }
        case 3: {
            unsigned int index = *(unsigned int*)&((ClubSelectionStorage*)self)->powerMode;
            result = kFloat_60_Club + (index ? kFloat_20_Club : kFloat_0_Club);
            break;
        }
        case 4: {
            Arr3 table = s_golfParameterModeOffsets;
            result += table.v[*(unsigned int*)&((ClubSelectionStorage*)self)->powerMode];
            break;
        }
        }
        break;
    case 3:
        if (((ClubSelectionStorage*)self)->longPutt) {
            const char* search = (const char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter);
            void* hit;
            if (node != 0)
                hit = strstr(((ClubParameterView*)node)->name, search);
            else
                hit = strstr(((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->name, search);
            result = hit ? kFloat_40_Club : kFloat_30_Club;
        }
        break;
    }

    void* base = ((ClubSelectionStorage*)self)->selected;
    if (((ClubParameterView*)base)->clubGroup != kClubPutter) {
        void* manager = g_sharedDoc;
        void* root = (*(void**)&AsCSharedDoc(manager)->golfDoc);
        result += CSharedDoc_ComputeAuxPartModifier(manager, AsCGolfDoc(root)->currentPlayer, 0);
    }
    base = ((ClubSelectionStorage*)self)->selected;
    if (((ClubParameterView*)base)->clubGroup != kClubPutter) {
        void* manager = g_sharedDoc;
        void* root = (*(void**)&AsCSharedDoc(manager)->golfDoc);
        unsigned char slot = AsCGolfDoc(root)->currentPlayer;
        void* state = (char*)manager + kPlayerSlotBytes * slot;
        int special = ItemInventory_IsSlot14ClubSetId(((PlayerInventorySlot*)state)->inventory);
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 3 && special == 0) {
            float factor = ((ShotAdjustmentState*)PartyItemSlots_GetInstance())->powerBonusRate;
            result = FP_FMADDS(result, factor, result);
        }
    }
    return result;
}

#pragma push
#pragma opt_propagation off
// Resolve a named or selected club's launch angle and apply lie-angle correction outside the closest tier.
float ClubSelection_GetLaunchAngle(ValueLookup* self, const char* key) {
    ValueNode* node;
    if (key != 0) {
        LockRef outer(self->lock);
        if (key == 0 || self->buckets == 0) {
            outer.release();
            node = 0;
        } else {
            unsigned int index;
            {
                void* inner = self->lock;
                OSLockMutex((OSMutex*)inner);
                int length = strlen(key);
                unsigned int first = (unsigned char)key[0] & 0x2A;
                int lastIndex = length > 0 ? length - 1 : 0;
                unsigned int last = (unsigned char)key[lastIndex] & 0x15;
                index = (first | last | ((unsigned int)length << 6)) & self->hashMask;
                OSUnlockMutex((OSMutex*)inner);
            }
            node = self->buckets[index];
            while (node != 0 || (outer.release(), node = 0, false)) {
                if (strcmp(node->key, key) == 0) {
                    ValueNode* nxt = node->payload;
                    outer.release();
                    node = nxt;
                    break;
                }
                node = node->same;
            };
        }
    } else {
        node = self->fallback;
    }
    if (self->mode == 4)
        return ((ClubParameterView*)node)->launchAngle;
    return FP_FMADDS_ADD_FIRST(((ClubParameterView*)node)->launchAngle, kFloat_0_06981317_GolfDocHelpers,
                               ((CGolfBall*)GolfBall__Fv())->m_initSpin);
}
#pragma pop

// Resolve launch speed by club group and distance tier, including power, equipment and party adjustments.
float ShotParameters_GetBaseLaunchSpeed(void* self) {
    // Phase 1: gather the player's equipment and optional party power rate.
    void* manager = g_sharedDoc;
    void* root = (*(void**)&AsCSharedDoc(manager)->golfDoc);
    unsigned char slot = AsCGolfDoc(root)->currentPlayer;
    void* state = (char*)manager + kPlayerSlotBytes * slot;
    int hasSpecialClubSet = ItemInventory_IsSlot14ClubSetId(((PlayerInventorySlot*)state)->inventory);
    float launchSpeed = ((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->launchSpeed;
    manager = g_sharedDoc;
    root = (*(void**)&AsCSharedDoc(manager)->golfDoc);
    float equipmentPowerModifier = CSharedDoc_ComputeAuxPartModifier(manager, AsCGolfDoc(root)->currentPlayer, 0);
    float* partyPowerRate = 0;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyMode && hasSpecialClubSet == 0)
        partyPowerRate = &((ShotAdjustmentState*)PartyItemSlots_GetInstance())->powerBonusRate;
    float powerModeDistanceBonus = kFloat_0_Club;
    // Phase 2: combine the selected power-mode offset with its earned bonus.
    InstanceTableReal* table = GetInstanceTable();
    if (((ClubSelectionStorage*)table)->powerMode) {
        float offset = hasSpecialClubSet ? kFloat_0_Club : ((ClubSelectionStorage*)self)->powerBonus;
        Arr3 offsets = s_golfParameterModeOffsets;
        float indexed = offsets.v[*(unsigned int*)&((ClubSelectionStorage*)self)->powerMode];
        powerModeDistanceBonus = offset + indexed;
    }
    ClubParameterView* clubParameters = (ClubParameterView*)((ClubSelectionStorage*)self)->selected;
    unsigned char clubGroup = ((ClubParameterView*)clubParameters)->clubGroup;
    // Phase 3: apply the group formula in its original floating-point order.
    // Distance divides a distance bonus into a dimensionless speed multiplier.
    switch (clubGroup) {
    case kClubWood: {
        float add;
        if (partyPowerRate) {
            float current = GolfParameter_ResolveFloat(self, 0) -
                            ((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->distanceYards;
            add = FP_FMADDS(kFloat_0_5_Camera, equipmentPowerModifier, current);
        } else {
            root = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            void* p = GolfPlayer_GetEntryByIndex(root, AsCGolfDoc(root)->currentPlayer);
            int value = ((PlayerShotStats*)p)->power;
            add = kFloat_2_golfParameters * FP_FMADDS(kFloat_0_5_Camera, equipmentPowerModifier, (float)value);
        }
        launchSpeed = launchSpeed * (kFloat_1_golfParameters +
                                     kFloat_1_5_GolfParameters * (powerModeDistanceBonus + add) /
                                         ((ClubParameterView*)((ClubSelectionStorage*)self)->selected)->distanceYards);
        break;
    }
    case kClubIron: {
        if (partyPowerRate) {
            float baseDistanceYards = clubParameters->distanceYards;
            float partyBonusRate = *partyPowerRate;
            float speedMultiplier = baseDistanceYards * partyBonusRate;
            speedMultiplier = powerModeDistanceBonus + speedMultiplier;
            speedMultiplier = speedMultiplier / baseDistanceYards;
            speedMultiplier = kFloat_1_golfParameters + speedMultiplier;
            launchSpeed = launchSpeed * speedMultiplier;
        } else {
            float baseDistanceYards = clubParameters->distanceYards;
            float speedMultiplier = powerModeDistanceBonus / baseDistanceYards;
            speedMultiplier = kFloat_1_golfParameters + speedMultiplier;
            launchSpeed = launchSpeed * speedMultiplier;
        }
        launchSpeed +=
            kFloat_1_3_GolfParameter * (launchSpeed * equipmentPowerModifier) / clubParameters->distanceYards;
        break;
    }
    case kClubWedge: {
        unsigned char distanceTier = ((ClubSelectionStorage*)self)->distanceTier;
        switch (distanceTier) {
        case kDistanceBelow10Yards:
        case kDistance10To15Yards:
        case kDistance15To28Yards: {
            float launchAngleRadians;
            if (distanceTier == kDistanceAtLeast58Yards)
                launchAngleRadians = ((ClubParameterView*)clubParameters)->launchAngle;
            else
                launchAngleRadians =
                    FP_FMADDS_ADD_FIRST(((ClubParameterView*)clubParameters)->launchAngle,
                                        kFloat_0_06981317_GolfDocHelpers, ((CGolfBall*)GolfBall__Fv())->m_initSpin);
            float specialAdd = ((ClubSelectionStorage*)self)->powerMode ? kFloat_28_GolfDocHelpers : kFloat_0_Club;
            launchSpeed =
                (kFloat_52_Club + specialAdd) *
                (kFloat_0_5_Camera + (kFloat_0_5_Camera * launchAngleRadians) / kWedgeSpeedReferenceAngleRadians);
            if (partyPowerRate)
                launchSpeed = FP_FMADDS_ADD_FIRST(launchSpeed, launchSpeed, *partyPowerRate);
            break;
        }
        case kDistance28To58Yards: {
            float launchAngleRadians;
            if (distanceTier == kDistanceAtLeast58Yards)
                launchAngleRadians = ((ClubParameterView*)clubParameters)->launchAngle;
            else
                launchAngleRadians =
                    FP_FMADDS_ADD_FIRST(((ClubParameterView*)clubParameters)->launchAngle,
                                        kFloat_0_06981317_GolfDocHelpers, ((CGolfBall*)GolfBall__Fv())->m_initSpin);
            float specialAdd =
                ((ClubSelectionStorage*)self)->powerMode ? kWedgeMidrangePowerSpeedAddition : kFloat_0_Club;
            launchSpeed =
                (kFloat_80_ClubSelection + specialAdd) *
                (kFloat_0_5_Camera + (kFloat_0_5_Camera * launchAngleRadians) / kWedgeSpeedReferenceAngleRadians);
            if (partyPowerRate)
                launchSpeed = FP_FMADDS_ADD_FIRST(launchSpeed, launchSpeed, *partyPowerRate);
            break;
        }
        case kDistanceAtLeast58Yards: {
            if (partyPowerRate) {
                float baseDistanceYards = clubParameters->distanceYards;
                float partyBonusRate = *partyPowerRate;
                float speedMultiplier = baseDistanceYards * partyBonusRate;
                speedMultiplier = powerModeDistanceBonus + speedMultiplier;
                speedMultiplier = speedMultiplier / baseDistanceYards;
                speedMultiplier = kFloat_1_golfParameters + speedMultiplier;
                launchSpeed = launchSpeed * speedMultiplier;
            } else {
                float baseDistanceYards = clubParameters->distanceYards;
                float speedMultiplier = powerModeDistanceBonus / baseDistanceYards;
                speedMultiplier = kFloat_1_golfParameters + speedMultiplier;
                launchSpeed = launchSpeed * speedMultiplier;
            }
            break;
        }
        }
        clubParameters = (ClubParameterView*)((ClubSelectionStorage*)self)->selected;
        launchSpeed +=
            ((ClubParameterView*)clubParameters)->launchSpeed * equipmentPowerModifier / clubParameters->distanceYards;
        break;
    }
    case kClubPutter: {
        if (((ClubSelectionStorage*)self)->longPutt)
            launchSpeed = strstr(((ClubParameterView*)clubParameters)->name,
                                 (char*)&clubAndPlayerStrings + CLUB_STRING_OFFSET(shortPutter))
                              ? kFloat_42_Club
                              : kFloat_36_Club;
        else
            launchSpeed = ((ClubParameterView*)clubParameters)->launchSpeed;
        break;
    }
    }
    return launchSpeed;
}

// Reset every progress-list payload value to unity under the list mutex.
void ShotProgressList_SetAllValuesToOne(ProgressList* self) {
    ProgressPayload* node;
    {
        WInstanceLock lock(self->lock);
        self->current = self->first;
        node = self->nextItem();
    }
    if (node != 0) {
        float value = kFloat_1_golfParameters;
        while (node != 0) {
            node->value = value;
            node = self->nextItem();
        }
    }
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Select a club record by registry name and copy its list index into the selection state.
void ClubSelection_SelectByName(LookupState* self, const char* key) {
    LookupState* s = self;
    void* mutex = s->lock;
    LockRef lock(mutex);
    LookupEntryNode* result;
    if (key == 0 || *(void**)&s->buckets == 0) {
        lock.release();
        result = 0;
    } else {
        int length;
        int lastIndex;
        int mask;
        unsigned int first;
        unsigned int last;
        unsigned int index;
        {
            void* m2 = s->lock;
            OSLockMutex((OSMutex*)m2);
            length = strlen(key);
            first = (unsigned char)key[0] & 0x2A;
            lastIndex = length > 0 ? length - 1 : 0;
            last = (unsigned char)key[lastIndex] & 0x15;
            index = (first | last | ((unsigned int)length << 6)) & s->hashMask;
            OSUnlockMutex((OSMutex*)m2);
        }
        result = (LookupEntryNode*)((LookupEntryNode**)*(void**)&s->buckets)[index];
        while (result != 0 || (lock.release(), result = 0, false)) {
            if (strcmp(result->key, key) == 0) {
                LookupEntryNode* nxt = result->payload;
                lock.release();
                result = nxt;
                break;
            }
            result = result->same;
        };
    }
    s->result = (EntryNode*)result;
    s->flag = *((unsigned char*)result + 9);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Select the club at a list index and store the selected record's index byte.
void ClubSelection_SetClubIndex(LookupState* self, unsigned int count) {
    EntryNode* node;
    {
        WInstanceLock outer(self->lock);
        self->current = self->end;
        node = self->nextNode();
    }
    unsigned char i = 0;
    if (count != 0) {
        void* lk = self->lock;
        while (i < count) {
            i++;
            node = self->nextNodeAt(lk);
        }
    }
    self->result = node;
    self->flag = ((ClubParameterView*)node)->clubIndex;
}

#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620EC", _ctors_u800E9BCC_0, CClock_RegisterType);
#pragma pop
