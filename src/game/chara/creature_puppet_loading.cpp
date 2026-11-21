typedef unsigned long new_size_t;
#include "classes/WPuppet.h"
// 0x8019962C: WPuppet key records (+0xC0) and named-bone table/count (+0x13C/+0x140).
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CParticle.h"
#include "classes/CCreature.h"

// Borrow creature storage through the recovered layout.
static inline CCreature* AsCCreature(void* creature) { return (CCreature*)creature; }

// Borrow particle storage, including byte-stepped views used by XYZ initialization.
static inline CParticle* AsCParticle(void* particle) { return (CParticle*)particle; }
// Translation unit 0x80198D58-0x80199DCC (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" { void* __nw__FUl(unsigned long); void __dl__FPv(void*); }
// Allocate creature storage through the game's scalar allocator.
inline void* operator new(new_size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();
// Construct a key payload in storage already owned by its list node.
inline void* operator new(new_size_t, void *p) {
    return p;
}

// Creature interface: the vtable pointer addresses the 8-byte header, so slot 37 is at offset 0x9C.
struct CreatureAttachmentInterface {
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03(); virtual void s04();
    virtual void s05(); virtual void s06(); virtual void s07(); virtual void s08(); virtual void s09();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24();
    virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30(); virtual void s31(); virtual void s32(); virtual void s33(); virtual void s34();
    virtual void s35(); virtual void s36();
    virtual void s37(void *entry);
};

extern "C" {

// Verified packed NPC string pool (global research round 7); arrays include terminators.
struct NpcStringPool {
    char vectorLengthError[20]; // +0x0 "vector length error"
    char exceptionTypes[55]; // +0x14 "!std::exception!!std::logic_error!!std::length_error!!"
    char santaTag[6]; // +0x4B "santa"
    char radiusToken[3]; // +0x51 "*r"
    char floatFormat[3]; // +0x54 "%f"
    char mozziModel[14]; // +0x57 "NPC_Mozzi.pet"
    char santaModel[14]; // +0x65 "NPC_Santa.pet"
    char flightTag[7]; // +0x73 "flight"
    char fighterModel[16]; // +0x7A "NPC_Fighter.pet"
    char ufoTag[4]; // +0x8A "ufo"
    char ufoModel[17]; // +0x8E "sand_house12.pet"
    char penguinWalkTag[12]; // +0x9F "penguinwalk"
    char penguinModel[18]; // +0xAB "NPC_penguin02.pet"
    char dragonTag[4]; // +0xBD "drg"
    char distanceToken[3]; // +0xC1 "*d"
    char dragonModel[15]; // +0xC4 "NPC_dragon.pet"
    char shipTag[5]; // +0xD3 "ship"
    char shipModel[13]; // +0xD8 "NPC_ship.pet"
    char npcTag[5]; // +0xE5 "npc_"
    char petNameFormat[7]; // +0xEA "%s.pet"
    char typeToken[6]; // +0xF1 "*type"
    char intFormat[3]; // +0xF7 "%d"
    char petToken[5]; // +0xFA "*pet"
    char stringFormat[3]; // +0xFF "%s"
    char countToken[5]; // +0x102 "*num"
    char blackSparrowModel[18]; // +0x107 "NPC_BTsparrow.pet"
    char velocityToken[5]; // +0x119 "*vel"
    char defaultAnimation[12]; // +0x11E "NPC_Default"
    char opacityKeyFormat[8]; // +0x12A "*opa %f"
    char effectToken[4]; // +0x132 "*fx"
    char effectArgumentsFormat[9]; // +0x136 "%s %s %s"
    char sprExtension[4]; // +0x13F "spr"
    char seqExtension[4]; // +0x143 "seq"
};
enum NpcKind {
    kNpcGull = 0, kNpcButterfly = 1, kNpcDuck = 2, kNpcMole = 3,
    kNpcDolphin = 4, kNpcCrab = 5, kNpcSparrow = 6, kNpcSanta = 7,
    kNpcFighter = 8, kNpcDisposable = 9, kNpcUnused = 10, kNpcUfo = 11,
    kNpcPenguin = 12, kNpcSailorPenguin = 13, kNpcDuckPenguin = 14,
    kNpcDragon = 15, kNpcBlackSparrow = 16, kNpcShip = 17,
    kNpcPolaPenguin1 = 18, kNpcPolaPenguin2 = 19
};
// Placement codes are the arguments used by this factory; their geometric
// interpretation belongs to the virtual placement method and is unresolved here.
enum NpcPlacementCode { kDefaultPlacement = 0, kDuckPlacement = 1, kButterflyPlacement = 4 };
enum CreatureLocalFlags { kBuildOpacityKeys = 1, kSuppressBoneEffects = 0x40000 };
// Field-derived byte views retain the original aliasing and pointer scheduling.
#define NPC_OFFSET(type, field) ((int)&((type*)0)->field)
struct NpcActorListView { char reserved00[0x18]; char actors[0x10]; };


struct OpacityKeyLinks {
    OpacityKeyLinks *next;
    OpacityKeyLinks *prev;
};

struct OpacityCleanupLinks {
    OpacityCleanupLinks *next;
    OpacityCleanupLinks *prev;
};

struct CreatureKeyAndPathStorage {
    unsigned int count;
    OpacityKeyLinks sentinel;
    void *targetPoints;
    unsigned int targetPointCount;
    unsigned int targetPointCapacity;
    unsigned int pathState;
    void *pathPoints;
    unsigned int pathPointCount;
};

struct CreatureResourceView {
    void *vtable;
    void *secondaryVtable;
    unsigned int reserved08[7];
    void *puppet;
    unsigned int reserved28[32];
    void *history;
    unsigned int reservedAC[2];
    CreatureKeyAndPathStorage list;
};

// Read the vector count through the original signed-word ABI view.
inline unsigned int count_80199414(unsigned int &value) {
    return *(int *)&value;
}

void *CCreature_LoadPuppetResource(void *, int, int);
void *ResourceManager_LoadByExtension(void *, int, void *, int, void *);
void ResourceManager_ReleaseObject(void *, void *, int);
void __dla__FPv(void *);
void appFreeUni__FPv(void *);
// Unlink and free the entire opacity-keyframe list, updating its count.
inline void clear_80199414(CreatureKeyAndPathStorage *list) {
    OpacityCleanupLinks *sentinel = (OpacityCleanupLinks *)&list->sentinel;
    OpacityCleanupLinks *node = (OpacityCleanupLinks *)list->sentinel.prev;
    if (node != sentinel) {
        OpacityCleanupLinks *last = sentinel->next;
        node->next->prev = last->prev;
        last->prev->next = node->next;
        while (node != sentinel) {
            OpacityCleanupLinks *old = node;
            node = node->prev;
            appFreeUni__FPv(old);
            --list->count;
        }
    }
}
void __dl__FPv(void *);
extern void *g_resourceManager;
extern void *g_sceneManager;
extern void *g_fx;
extern char __vt__9CCreature[];

// Load and cache the creature's puppet, scheduling its resource preload.
void *CCreature_LoadPuppet(void *self, int arg) {
    void *result = CCreature_LoadPuppetResource(self, arg, 1);
    *(void **)&AsCCreature(self)->puppet = result;
    // CCreature_LoadPuppet: 0x24
    return result;
}

// Release the puppet, movement history, opacity-key list and target/path vectors; optionally delete the creature.
void *CCreature_dtor(CreatureResourceView *self, int doDelete) {
    if (self != 0) {
        self->vtable = __vt__9CCreature;
        self->secondaryVtable = __vt__9CCreature + 0x0C;
        // Release manager-owned puppet references before freeing local allocations.
        if (g_resourceManager != 0) {
            if (self->puppet) {
                ResourceManager_ReleaseObject(g_resourceManager, self->puppet, 0);
                self->puppet = 0;
            }
        }
        if (self->history != 0) {
            __dla__FPv(self->history);
            self->history = 0;
        }
        {
            OpacityKeyLinks *node = self->list.sentinel.prev;
            OpacityKeyLinks *sentinel = &self->list.sentinel;
            int n2 = *(int *)&self->list.pathPointCount;
            int c4snap = (int)self->list.targetPointCount;
            self->list.pathPointCount = n2 - self->list.pathPointCount;
            self->list.targetPointCount = c4snap - (int)self->list.targetPointCount;
            if (node != sentinel) {
                OpacityKeyLinks *last = sentinel->next;
                node->next->prev = last->prev;
                last->prev->next = node->next;
                while (node != sentinel) {
                    OpacityKeyLinks *old = node;
                    node = node->prev;
                    appFreeUni__FPv(old);
                    self->list.count--;
                }
            }
        }
        // Empty/free each vector, then run the list's (now empty) base cleanup.
        // Repeated address tests retain the original inlined destructor shape.
        if (&self->list.pathPoints) {
            if (&self->list.pathPoints) {
                if (&self->list.pathPoints) {
                    if (&self->list.pathPoints) {
                        if (self->list.pathPoints) {
                            self->list.pathPointCount -= count_80199414(self->list.pathPointCount);
                            appFreeUni__FPv(self->list.pathPoints);
                        }
                    }
                }
            }
        }
        if (&self->list.targetPoints) {
            if (&self->list.targetPoints) {
                if (&self->list.targetPoints) {
                    if (&self->list.targetPoints) {
                        if (self->list.targetPoints != 0) {
                            self->list.targetPointCount -= count_80199414(self->list.targetPointCount);
                            appFreeUni__FPv(self->list.targetPoints);
                        }
                    }
                }
            }
        }
        if (&self->list) {
            if (&self->list) {
                if (&self->list) {
                    if (&self->list) {
                        clear_80199414(&self->list);
                    }
                }
            }
        }
        if (doDelete > 0)
        __dl__FPv(self);
    }
    return self;
}

#include "sdk_protos.h"
int String_CompareCString(const char *, const char *);
void FxAttachment_Init(void *);
void CFx_CreateAttachedEffect(const char *, void *, const char *, int, int, int, void *);
extern char npcResourceStrings[];
extern float kFloat_2_805DF2B0;

void BorrowedNameRef_Set(void *, const char *);
int String_ScanTokensAfterSubstring(const char *, const char *, const char *, ...);
void CGull_Ctor(void *, const char *);
void CButterfly_Ctor(void *, const char *);
void CDuck_Ctor(void *, const char *, int);
void CMole_Ctor(void *, const char *);
void CDolphin_Ctor(void *, const char *);
void CCrab_Ctor(void *, const char *);
void CSparrow_Ctor(void *, const char *, int);
void CFighter_Ctor(void *, const char *);
void CDisposable_Ctor(void *, const char *);
void ntPenguin_Sailor_Ctor(void *, const char *);
void ntPenguin_Duck_Ctor(void *, const char *, int);
void ntNpc_BlackSparrow_Ctor(void *, const char *);
void ntNpc_PolaPenguin1_Ctor(void *, const char *, float);
void ntNpc_PolaPenguin2_Ctor(void *, const char *);
void CCreature_SetKind(void *, int);
void NpcActorList_PushBack(void *, void *);

struct NpcVtableView { void *vtable; };

struct NpcPlacementInterface {
    virtual void s00(); virtual void s01(); virtual void s02(); virtual void s03(); virtual void s04();
    virtual void s05(); virtual void s06(); virtual void s07(); virtual void s08(); virtual void s09();
    virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14();
    virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24();
    virtual void s25(); virtual void s26(); virtual void s27(); virtual void s28(); virtual void s29();
    virtual void s30();
    // Slot 31 (+0x84 including MWCC's 8-byte header): parent + placement code.
    // Shared symbol m is retained; the local adapter below names its factory role.
    virtual void m(void *, int);
};

// Apply the factory's placement code relative to the borrowed parent object.
// Modes 0/1/4 are established at the call sites; their geometry remains unresolved.
static inline void PlaceCreatedNpc(void* actor, void* parent, int placementCode) {
    ((NpcPlacementInterface*)actor)->m(parent, placementCode);
}

// Each storage wrapper reserves the derived object's exact byte size and calls
// its real constructor. `new` retains constructor-failure cleanup; these are
// opaque storage, not inferred complete class definitions. p is the model name.
// Construct the gull in its 0x11c-byte allocation.
struct GullActorStorage { char data[0x11c]; inline GullActorStorage(const char* p){ CGull_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct ButterflyActorStorage { char data[0x11c]; inline ButterflyActorStorage(const char* p){ CButterfly_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct DuckActorStorage { char data[0x130]; inline DuckActorStorage(const char* p,int n){ CDuck_Ctor(this,p,n); } };
// Construct the derived actor in its exact-sized allocation.
struct MoleActorStorage { char data[0x11c]; inline MoleActorStorage(const char* p){ CMole_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct DolphinActorStorage { char data[0x11c]; inline DolphinActorStorage(const char* p){ CDolphin_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct CrabActorStorage { char data[0x11c]; inline CrabActorStorage(const char* p){ CCrab_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct SparrowActorStorage { char data[0x128]; inline SparrowActorStorage(const char* p,int n){ CSparrow_Ctor(this,p,n); } };
// Construct the derived actor in its exact-sized allocation.
struct FighterActorStorage { char data[0x124]; inline FighterActorStorage(const char* p){ CFighter_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct DisposableActorStorage { char data[0x134]; inline DisposableActorStorage(const char* p){ CDisposable_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct SailorPenguinActorStorage { char data[0x124]; inline SailorPenguinActorStorage(const char* p){ ntPenguin_Sailor_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct DuckPenguinActorStorage { char data[0x134]; inline DuckPenguinActorStorage(const char* p,int n){ ntPenguin_Duck_Ctor(this,p,n); } };
// Construct the derived actor in its exact-sized allocation.
struct BlackSparrowActorStorage { char data[0x11c]; inline BlackSparrowActorStorage(const char* p){ ntNpc_BlackSparrow_Ctor(this,p); } };
// Construct the derived actor in its exact-sized allocation.
struct PolaPenguin1ActorStorage { char data[0x11c]; inline PolaPenguin1ActorStorage(float v,const char* p){ ntNpc_PolaPenguin1_Ctor(this,p,v); } };
// Construct the derived actor in its exact-sized allocation.
struct PolaPenguin2ActorStorage { char data[0x120]; inline PolaPenguin2ActorStorage(const char* p){ ntNpc_PolaPenguin2_Ctor(this,p); } };

#pragma push
#pragma opt_common_subs on
#pragma opt_strength_reduction off
#pragma opt_lifetimes on
#pragma opt_propagation off
// Parse *type, *pet, *num and *vel tokens, construct matching creatures and attach/register them.
void CNPC_CreateActorsFromDefinition(void *selfIn, void *parentIn, const char *definition) {
    int placementModeOrIndex; // Reused as the count-loop index in multi-actor cases.
    void *npcActor;
    void *parentPlacement;
    char name[0x20];
    void *object;
    unsigned int kind;
    int count;
    const char *parser;
    float velocity;
    // Decode the borrowed definition. Required *type/*pet tokens and their
    // buffers are trusted, as in the original; failed scans are not validated.
    object = 0;
    npcActor = selfIn;
    parentPlacement = parentIn;
    placementModeOrIndex = 0;
    BorrowedNameRef_Set(&parser, definition);
    String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, typeToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &kind);
    String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, petToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, stringFormat)), name);
    // Expand *num either into separate registered actors (gull/butterfly and
    // penguin families) or one constructor's group argument (duck/sparrow).
    // parentIn is borrowed; the actor list retains each constructed pointer.
    do {
        switch (kind) {
            case kNpcGull:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                    object = new GullActorStorage(name);
                    PlaceCreatedNpc(object, parentPlacement, kDefaultPlacement);
                    CCreature_SetKind(object, kind);
                    NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
                }
            } else {
                object = new GullActorStorage(name);
                break;
            }
            continue;
            case kNpcButterfly:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                    object = new ButterflyActorStorage(name);
                    PlaceCreatedNpc(object, parentPlacement, kButterflyPlacement);
                    CCreature_SetKind(object, kind);
                    NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
                }
            } else {
                object = new ButterflyActorStorage(name);
                break;
            }
            continue;
            case kNpcDuck:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                object = new DuckActorStorage(name, count + 1);
            } else {
                object = new DuckActorStorage(name, 0);
            }
            placementModeOrIndex = kDuckPlacement;
            break;
            case kNpcMole:
            object = new MoleActorStorage(name);
            break;
            case kNpcDolphin:
            object = new DolphinActorStorage(name);
            break;
            case kNpcCrab:
            object = new CrabActorStorage(name);
            break;
            case kNpcSparrow:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                object = new SparrowActorStorage(name, count);
            }
            break;
            case kNpcSanta:
            case kNpcUnused:
            break;
            case kNpcFighter:
            object = new FighterActorStorage(name);
            break;
            case kNpcDisposable:
            object = new DisposableActorStorage(name);
            break;
            case kNpcSailorPenguin:
            count = 1;
            String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count);
            for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                object = new SailorPenguinActorStorage(name);
                PlaceCreatedNpc(object, parentPlacement, kDefaultPlacement);
                CCreature_SetKind(object, kind);
                NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
            }
            continue;
            case kNpcDuckPenguin:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                object = new DuckPenguinActorStorage(name, count + 1);
            } else {
                object = new DuckPenguinActorStorage(name, 0);
            }
            placementModeOrIndex = kDuckPlacement;
            break;
            case kNpcBlackSparrow:
            if (String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count)) {
                for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                    object = new BlackSparrowActorStorage((npcResourceStrings + NPC_OFFSET(NpcStringPool, blackSparrowModel)));
                    PlaceCreatedNpc(object, parentPlacement, kDefaultPlacement);
                    CCreature_SetKind(object, kind);
                    NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
                }
            } else {
                object = new BlackSparrowActorStorage((npcResourceStrings + NPC_OFFSET(NpcStringPool, blackSparrowModel)));
                break;
            }
            continue;
            case kNpcPolaPenguin1: {
                velocity = kFloat_2_805DF2B0;
                String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, velocityToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, floatFormat)), &velocity);
                count = 1;
                String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count);
                for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                    object = new PolaPenguin1ActorStorage(velocity, name);
                    PlaceCreatedNpc(object, parentPlacement, kDefaultPlacement);
                    CCreature_SetKind(object, kind);
                    NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
                }
                continue;
            }
            case kNpcPolaPenguin2:
            count = 1;
            String_ScanTokensAfterSubstring((const char *)&parser, (npcResourceStrings + NPC_OFFSET(NpcStringPool, countToken)), (npcResourceStrings + NPC_OFFSET(NpcStringPool, intFormat)), &count);
            for (placementModeOrIndex = 0; placementModeOrIndex < count; ++placementModeOrIndex) {
                object = new PolaPenguin2ActorStorage(name);
                PlaceCreatedNpc(object, parentPlacement, kDefaultPlacement);
                CCreature_SetKind(object, kind);
                NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
            }
            // Fall through: default must share this return block's jump-table target.
            default:
            return;
        }
        // Single-object cases share attachment and registration. The reused
        // placementModeOrIndex is a placement code here, a counter in loops.
        // Unsupported/required-token-missing cases retain the original behavior;
        // do not infer that a non-null object is guaranteed for every type code.
        PlaceCreatedNpc(object, parentPlacement, placementModeOrIndex);
        CCreature_SetKind(object, kind);
        NpcActorList_PushBack(((char *)npcActor + NPC_OFFSET(NpcActorListView, actors)), &object);
    } while (0);
}
#pragma pop

void AABB_Copy(void *, const void *);
float Random_RangeFloat(int, float, float);
void Matrix_SetAxisRotation(void *, float, int);
void Matrix_CopyAffine(void *, void *);
void *CQuadTree_GetInstance(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void *, void *, void *, int, int, int);
void __as__7WVectorFRC7WVector(void *, void *);
void *__nwa__FUl(unsigned int);
extern float kFloat_neg180_Creature;
extern float kFloat_180_Creature;
extern float kFloat_0_017453292_Creature;
extern float kFloat_1_Creature;
extern const float kFloat_0_Creature;
extern const float kFloat_0_7853982_Creature;
extern float kCreatureAnimationFramesPerSecond;
extern float kFloat_2_55_CreatureAnimation;
extern const float kFloat_1_0471976_Creature;
extern double kDouble_SignedIntConversionBias_Creature;

typedef Vec3f Vec3_80199AAC;

#pragma push
#pragma opt_common_subs off
enum ParticleGridConstants {
    kSkipRandomGridInitialization = 1, kParticleAxisCount = 3,
    kCellIndexBytes = 4, kFlatCellHistorySlots = 2, kLayeredCellHistorySlots = 4
};
enum ParticlePlacementFlags { kPlaceOnGround = 1, kPlaceOnGroundAlternate = 2, kUseVerticalCells = 4 };
// Choose a random pose in the bounds, optionally project onto ground and initialize the movement-grid history.
void CCreature_InitRandomGridCell(void *self, const void *worldBounds, unsigned int flags) {
    char *axisByteCursor;
    int axisIndex;
    int historySlots;
    int cellX;
    int cellY;
    int cellZ;
    int flattenedCell;
    int historyByteOffset;
    int historySlotIndex;
    float yawRadians;
    float lowerBoundWeight;
    float unitWeight;
    float cellCountXFloat;
    float cellCountYFloat;
    float cellCountZFloat;
    float rotation[12];
    void *newCellHistory;
    // Phase 1: copy world bounds even when the state bit skips pose/grid initialization.
    // targetCellSize must be positive and bounds must permit nonzero cell counts.
    AABB_Copy(&AsCParticle(self)->boundsMinX, worldBounds);
    // 0xC: minimum and maximum bounds
    if ((AsCParticle(self)->placementStateFlags & kSkipRandomGridInitialization) == 0) {
        yawRadians = kFloat_0_017453292_Creature * Random_RangeFloat(-1, kFloat_neg180_Creature, kFloat_180_Creature);
        Matrix_SetAxisRotation(rotation, yawRadians, 1);
        Matrix_CopyAffine(AsCParticle(self)->transform, rotation);
        // Axis cursor advances one float at a time: boundsMinX/MaxX and
        // transform[9] become Y then Z through the same byte-stepped view.
        unitWeight = kFloat_1_Creature;
        axisByteCursor = (char *)self;
        axisIndex = 0;
        do {
            lowerBoundWeight = Random_RangeFloat(-1, kFloat_0_Creature, kFloat_1_Creature);
            axisIndex += 1;
            float inverseBlend = unitWeight - lowerBoundWeight;
            AsCParticle(axisByteCursor)->transform[9] = lowerBoundWeight * AsCParticle(axisByteCursor)->boundsMinX + inverseBlend * AsCParticle(axisByteCursor)->boundsMaxX;
            // 0x4C
            axisByteCursor += 4;
        } while (axisIndex < kParticleAxisCount);
        // Phase 2: project the world position. If both flags are set, run both
        // ground queries in this order; the alternate query supplies the final position.
        if (flags & kPlaceOnGround) {
            Vec3_80199AAC b;
            Vec3_80199AAC a = *(Vec3_80199AAC *)&AsCParticle(self)->transform[9];
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&b, (void *)CQuadTree_GetInstance(), &a, 0, 0, 1);
            *(WVector*)&AsCParticle(self)->transform[9] = *(WVector*)&b;
        }
        if (flags & kPlaceOnGroundAlternate) {
            Vec3_80199AAC b;
            Vec3_80199AAC a = *(Vec3_80199AAC *)&AsCParticle(self)->transform[9];
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&b, (void *)CQuadTree_GetInstance(), &a, 0, 0, 0);
            *(WVector*)&AsCParticle(self)->transform[9] = *(WVector*)&b;
        }
        // Phase 3: commit the projected world position and size the XYZ grid. History stores flattened cells
        // x + countX*(z + y*countZ), repeated in every initial history slot.
        *(WVector*)&AsCParticle(self)->gridPositionX = *(WVector*)&AsCParticle(self)->transform[9];
        // 0x64, 0x4C
        cellCountXFloat = (float)ceil((double)((AsCParticle(self)->boundsMaxX - AsCParticle(self)->boundsMinX) / AsCParticle(self)->targetCellSize));
        if (flags & kUseVerticalCells) {
            cellCountYFloat = (float)ceil((double)(((*(float*)&AsCParticle(self)->unk_01c) - AsCParticle(self)->boundsMinY) / AsCParticle(self)->targetCellSize));
        } else {
            cellCountYFloat = kFloat_1_Creature;
        }
        cellCountZFloat = (float)ceil((double)(((*(float*)&AsCParticle(self)->unk_020) - AsCParticle(self)->boundsMinZ) / AsCParticle(self)->targetCellSize));
        AsCParticle(self)->gridCountX = (int)cellCountXFloat;
        AsCParticle(self)->gridCountY = (int)cellCountYFloat;
        AsCParticle(self)->gridCountZ = (int)cellCountZFloat;
        AsCParticle(self)->cellSizeX = (AsCParticle(self)->boundsMaxX - AsCParticle(self)->boundsMinX) / cellCountXFloat;
        AsCParticle(self)->cellSizeY = ((*(float*)&AsCParticle(self)->unk_01c) - AsCParticle(self)->boundsMinY) / cellCountYFloat;
        AsCParticle(self)->cellSizeZ = ((*(float*)&AsCParticle(self)->unk_020) - AsCParticle(self)->boundsMinZ) / cellCountZFloat;
        // Phase 4: allocate 4 history slots for a multi-layer grid, otherwise 2.
        // This replaces cellIndices without freeing an old array: callers must
        // arrange first initialization or dispose previous history themselves.
        historySlots = AsCParticle(self)->gridCountY > 1 ? kLayeredCellHistorySlots : kFlatCellHistorySlots;
        AsCParticle(self)->cellIndexCount = historySlots;
        newCellHistory = __nwa__FUl(historySlots * kCellIndexBytes);
        AsCParticle(self)->cellIndices = newCellHistory;
        // Zero-based cell coordinates are measured from the minimum world bound.
        // No clamping is performed at the maximum edge; retain the RNG/bounds contract.
        cellX = (int)((AsCParticle(self)->gridPositionX - AsCParticle(self)->boundsMinX) / AsCParticle(self)->cellSizeX);
        cellY = (int)((AsCParticle(self)->gridPositionY - AsCParticle(self)->boundsMinY) / AsCParticle(self)->cellSizeY);
        cellZ = (int)((AsCParticle(self)->gridPositionZ - AsCParticle(self)->boundsMinZ) / AsCParticle(self)->cellSizeZ);
        flattenedCell = AsCParticle(self)->gridCountX * (cellZ + cellY * AsCParticle(self)->gridCountZ) + cellX;
        historySlotIndex = 0;
        historyByteOffset = 0;
        while (historySlotIndex < AsCParticle(self)->cellIndexCount) {
            historySlotIndex += 1;
            *(int *)((*(char**)&AsCParticle(self)->cellIndices) + historyByteOffset) = flattenedCell;
            historyByteOffset += kCellIndexBytes;
        }
    }
}
#pragma pop

void *appMallocDummy__FUl(unsigned int);
void *WPuppet_FindAnimationRangeByName(void *, const char *);
void *WPuppet_FindKeyRange(void *, int, int, int *, int *);
int CString_Compare(void *, const char *);
void ResourcePreloadQueue_AddRequest(void *, int, void *, float, void *, float, int, void *, int, int);

struct CreatureBoneEffect {
    struct BoneTransform {
        char reserved00[0x74]; unsigned int flags; float worldMatrix[12];
    }* bone;
    struct Metadata {
        char reserved00[0x18]; char attachmentData[0x10]; const char* effectDefinition;
    }* metadata;
};
struct AnimationRangeView { float start, end; char reserved08[0x10]; void* name; };
struct PuppetKeyView { int frame; unsigned int reserved; char* text; };

// Parse a named-bone effect definition and attach supported .spr or .seq resources to the bone.
void CCreature_AttachBoneMetadataEffect(void *self, void *boneEntry) {
    if (boneEntry == 0)
    return;
    if (((CreatureBoneEffect*)boneEntry)->metadata->effectDefinition == 0)
    return;
    const char* effectDefinition = strstr(((CreatureBoneEffect*)boneEntry)->metadata->effectDefinition, (npcResourceStrings + NPC_OFFSET(NpcStringPool, effectToken)));
    char attachmentName[0x20];
    // sscanf %s targets; original layout: command at +8 (8 bytes), attachmentName at +0x10 (0x20 bytes), effectName at +0x30
    char command[8];
    char effectName[0x20];
    char temporary[0x28];
    sscanf(effectDefinition, (npcResourceStrings + NPC_OFFSET(NpcStringPool, effectArgumentsFormat)), command, effectName, attachmentName);
    if (((CreatureBoneEffect*)boneEntry)->bone == 0 || (((CreatureBoneEffect*)boneEntry)->bone->flags & kSuppressBoneEffects) != 0)
    return;
    char *dot = strchr(effectName, '.');
    if (dot == 0 || strlen(dot) < 4)
    return;
    if (String_CompareCString(dot + 1, (npcResourceStrings + NPC_OFFSET(NpcStringPool, sprExtension))) != 0 && String_CompareCString(dot + 1, (npcResourceStrings + NPC_OFFSET(NpcStringPool, seqExtension))) != 0)
    return;
    FxAttachment_Init(&temporary);
    *(void **)(temporary + 8) = ((CreatureBoneEffect*)boneEntry)->bone->worldMatrix;
    *(void **)(temporary + 0x18) = ((CreatureBoneEffect*)boneEntry)->metadata->attachmentData;
    CFx_CreateAttachedEffect((const char *)g_fx, temporary, effectName, 0, 0, 0, ((CreatureBoneEffect*)boneEntry)->metadata);
}

// Two-float ABI overlay with two distinct uses below:
// opacity key: v = *opa percent * 2.55 (0..255 scale), t = frame/FPS (seconds);
// animation range: v/t = start/end in seconds. It is not one shared logical type.
typedef struct Pair { float v; float t; } Pair;

// Bitwise construction view of an opacity key, never integer arithmetic:
// a carries the first float's bits, b carries the second float's bits.
// Retain the integer-copy form for the original placement-construction codegen.
typedef struct Raw {
    int a; int b;
} Raw;

// Read/write the key's alpha value on the 0..255 scale (parsed as percent first).
static inline float& OpacityKeyValue255(Pair& key) { return key.v; }
// Read/write the key timestamp in seconds, derived from integer frame / FPS.
static inline float& OpacityKeyTimeSeconds(Pair& key) { return key.t; }
// Read a borrowed animation range's start time in seconds.
static inline float AnimationStartSeconds(const Pair* range) { return range->v; }
// Read a borrowed animation range's end time in seconds.
static inline float AnimationEndSeconds(const Pair* range) { return range->t; }

struct SmallNode {
    SmallNode *next;
    SmallNode *prev;
    Raw val;
};
// Circular doubly-linked list: sentinel is embedded in CCreature, payload starts
// at +8; each 16-byte allocation owns only its two-float key. appFreeUni frees it.

struct NodeGuard_8019962C {
    SmallNode *list;
    SmallNode *ptr;
    // Retain the unlinked node until insertion commits ownership to the list.
    NodeGuard_8019962C(SmallNode *s, SmallNode *p) : list(s), ptr(p) {}
    // Free an uncommitted allocation on scope exit.
    ~NodeGuard_8019962C() { if (ptr) appFreeUni__FPv(ptr); }
};

#pragma push
#pragma opt_common_subs on
struct Alloc_8019962C {
    char dummy;
    // Initialize the allocator tag used by this list specialization.
    Alloc_8019962C() : dummy(0) {}
    // No allocator state requires destruction.
    ~Alloc_8019962C() {}
};
struct Iter_8019962C {
    SmallNode *ptr;
    // Wrap a borrowed list-node address.
    Iter_8019962C(SmallNode *p) : ptr(p) {}
    // Iterator destruction does not free the pointed-to node.
    ~Iter_8019962C() {}
};
// Insert an opacity/time key at the front of the creature's circular keyframe list.
inline void insert_8019962C(void *self, const Pair &pair, const Alloc_8019962C &) {
    SmallNode *node;
    SmallNode *sentinel;
    sentinel = (SmallNode *)((char *)self + NPC_OFFSET(CCreature, opacityKeyframeNext));
    node = (SmallNode *)appMallocDummy__FUl(0x10);
    Iter_8019962C allocated(node);
    Iter_8019962C result(node);
    NodeGuard_8019962C guard(sentinel, node);
    try {
        new (&node->val) Raw(*(const Raw *)&pair);
        guard.ptr = 0;
        sentinel->next->prev = node;
        node->next = sentinel->next;
        sentinel->next = node;
        node->prev = sentinel;
        ++*(unsigned int *)&AsCCreature(self)->opacityKeyframeCount;
    } catch (...) {
        appFreeUni__FPv(node);
    }
}

// Deactivate animation and free every opacity-keyframe node.
inline void clear_8019962C(void *self) {
    OpacityKeyLinks *node;
    OpacityKeyLinks *sentinel;
    OpacityKeyLinks *old;
    node = *(OpacityKeyLinks **)&AsCCreature(self)->opacityKeyframePrevious;
    sentinel = (OpacityKeyLinks *)((char *)self + NPC_OFFSET(CCreature, opacityKeyframeNext));
    AsCCreature(self)->animationActive = 0;
    if (node != sentinel) {
        OpacityKeyLinks *first = sentinel->next;
        node->next->prev = first->prev;
        first->prev->next = node->next;
        while (node != sentinel) {
            old = node;
            node = node->prev;
            appFreeUni__FPv(old);
            --*(unsigned int *)&AsCCreature(self)->opacityKeyframeCount;
        }
    }
}

// Load a puppet, choose its default animation, build opacity keys and notify each named-bone attachment hook.
void *CCreature_LoadPuppetResource(void *self, int resourceId, int queuePreload) {
    char *handle;
    int requestedId;
    Pair opacityKey;
    SmallNode *sentinel;
    SmallNode *node;
    int firstKey;
    int lastKey;
    void *obj;
    const Pair *animationTimeRange;
    char *name;
    SmallNode *old;
    float lastFrame;
    float firstFrame;
    void *res;
    int keyByteOffset;
    int keyIndex;
    int parsedValues;
    unsigned int boneByteOffset;
    unsigned int boneIndex;
    char *boneEntry;
    // Acquire the resource (null means load failure). The returned handle stays
    // managed by the resource manager; the caller caches/releases it on self.
    requestedId = resourceId;
    void *nullArg = 0;
    handle = (char *)ResourceManager_LoadByExtension(g_resourceManager, requestedId, nullArg, 1, nullArg);
    if (handle == 0)
    return 0;
    if (queuePreload != 0) {
        ResourcePreloadQueue_AddRequest(g_sceneManager, 4, handle, kFloat_1_Creature, self, kFloat_0_Creature, requestedId, AsCCreature(self)->transform, 0, 0);
    }
    AsCCreature(self)->bankAngle = kFloat_0_Creature;
    AsCCreature(self)->recordPath = 0;
    AsCCreature(self)->opacity = 0xFF;
    AsCCreature(self)->pathCursor = 0;
    *(float*)&AsCCreature(self)->pad_080 = kFloat_0_7853982_Creature;
    // Reuse the named range or choose a random starting time within its seconds.
    name = (npcResourceStrings + NPC_OFFSET(NpcStringPool, defaultAnimation));
    if (handle != 0) {
        obj = *(void **)&AsCCreature(self)->animationRange;
        if (obj == 0 || CString_Compare(((AnimationRangeView*)obj)->name, name) != 0) {
            animationTimeRange = (const Pair *)WPuppet_FindAnimationRangeByName(handle, name);
            if (animationTimeRange != 0) {
                *(const Pair **)&AsCCreature(self)->animationRange = animationTimeRange;
                float low = AnimationStartSeconds(animationTimeRange);
                float high = AnimationEndSeconds(animationTimeRange);
                AsCCreature(self)->animationTime = Random_RangeFloat(-1, low, high);
            } else {
                AsCCreature(self)->animationTime = kFloat_0_Creature;
            }
        }
    }
    // Bit 0 selects metadata-driven opacity keys and clears animationActive.
    // This branch assumes a valid animationRange, even if lookup above failed.
    // Cover all integer key frames intersecting [start,end] seconds inclusively.
    if ((*(unsigned int *)&AsCCreature(self)->flags & kBuildOpacityKeys) != 0) {
        clear_8019962C(self);
        obj = *(void **)&AsCCreature(self)->animationRange;
        lastFrame = (float)ceil(kCreatureAnimationFramesPerSecond * ((AnimationRangeView*)obj)->end);
        obj = *(void **)&AsCCreature(self)->animationRange;
        firstFrame = (float)floor(kCreatureAnimationFramesPerSecond * ((AnimationRangeView*)obj)->start);
        res = WPuppet_FindKeyRange(handle, (int)firstFrame, (int)lastFrame, &firstKey, &lastKey);
        if (res != 0) {
            keyIndex = firstKey;
            keyByteOffset = keyIndex * 12;
            while (keyIndex <= lastKey) {
                parsedValues = sscanf(((PuppetKeyView*)((char*)AsWPuppet(handle)->keyRecords + keyByteOffset))->text, (npcResourceStrings + NPC_OFFSET(NpcStringPool, opacityKeyFormat)), &OpacityKeyValue255(opacityKey));
                if (parsedValues != 0) {
                    // v is still the parsed percent here; store it back on the 0..255 scale.
                    opacityKey.v = opacityKey.v * kFloat_2_55_CreatureAnimation;
                    OpacityKeyTimeSeconds(opacityKey) = (float)((PuppetKeyView*)((char*)AsWPuppet(handle)->keyRecords + keyByteOffset))->frame / kCreatureAnimationFramesPerSecond;
                    insert_8019962C(self, opacityKey, Alloc_8019962C());
                }
                keyByteOffset += 12;
                ++keyIndex;
            }
        }
    } else {
        AsCCreature(self)->animationActive = 1;
        AsCCreature(self)->turnRate = kFloat_1_0471976_Creature;
    }
    // Attach effects after key setup; the virtual hook receives borrowed bone entries.
    boneByteOffset = 0;
    for (boneIndex = 0; boneIndex < AsWPuppet(handle)->namedBoneCount; ++boneIndex) {
        boneEntry = (char*)AsWPuppet(handle)->namedBones + boneByteOffset;
        if (boneEntry != 0) {
            ((CreatureAttachmentInterface *)self)->s37(boneEntry);
        }
        boneByteOffset += 8;
    }
    return handle;
}
#pragma pop

} // extern "C"

// Private views must retain the original 32-bit ABI offsets.
typedef char u80198D58_layout_check_0[(NPC_OFFSET(NpcStringPool, defaultAnimation) == 0x11E) ? 1 : -1];
typedef char u80198D58_layout_check_1[(NPC_OFFSET(NpcStringPool, opacityKeyFormat) == 0x12A) ? 1 : -1];
typedef char u80198D58_layout_check_2[(NPC_OFFSET(CreatureBoneEffect::Metadata, effectDefinition) == 0x28) ? 1 : -1];
