#include "MSL/error_message.h"
#include "classes/CCreature.h"
#include "classes/CSharedDoc.h"
#include "game/types.h"
#include "game/wvector.h"
// Translation unit 0x801979C4-0x80198D58 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct SharedNpcHoleCodes { char reserved00[0x850]; unsigned int holeCodes[18]; };
// Return caller-provided placement storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

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
// Field-derived byte views retain the original aliasing and pointer scheduling.
#define NPC_OFFSET(type, field) ((int)&((type*)0)->field)
struct NpcActorListView { char reserved00[0x18]; char actors[0x10]; };


struct NpcActorLink { NpcActorLink* prev; NpcActorLink* next; void* object; };
struct NpcActorListHead { char reserved00[0x1c]; NpcActorLink* head; NpcActorLink* first; };
typedef Vec3f NpcPoint;
struct NpcBounds { float lower[3], upper[3]; };
// Test inclusive world-space xyz bounds against a borrowed point.
inline bool Fn80197D30Contains(const NpcBounds& a, const NpcPoint* r) {
    return a.lower[0]<=r->x && r->x<=a.upper[0] && a.lower[1]<=r->y && r->y<=a.upper[1] && a.lower[2]<=r->z && r->z<=a.upper[2];
}
struct Fn80197D30Vec {
    struct { unsigned int x,y,z; } value;
    // Copy the xyz bit patterns while retaining the original exception boundary.
    void assign(const Fn80197D30Vec& v) {
        try { *this=v; } catch (...) { throw; }
    }
    // Copy-construct the three-word point payload.
    Fn80197D30Vec(const Fn80197D30Vec& v) { assign(v); }
};
struct NpcCreatureView {
    virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34();
    virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54();
    virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64();
    virtual void v68(); virtual void v6C(); virtual void v70(); virtual void v74();
    virtual void v78(); virtual void v7C(); virtual void v80(); virtual void v84();
    virtual void v88(); virtual void commit(); virtual void v90(); virtual void update(int);
    unsigned int secondaryVtable, flags;
    NpcBounds bounds;
    char reserved24[0x34];
    float animationFrame;
    int kind;
    unsigned int animationRange;
    Fn80197D30Vec targetPosition;
    char reserved70[8];
    float bankAngle;
    char reserved7C[0x31];
    unsigned char animationActive;
    char reservedAE[0x22];
    Fn80197D30Vec* pathPoints;
    unsigned int pathPointCount, pathPointCapacity;
    unsigned char pathCursor;
    char reservedDD[3];
    char savedMatrix[0x30];
    float savedAnimationFrame, savedBankAngle;
    unsigned char savedAnimationActive, recordPath;
    // Return a value copy of the creature world-space bounds.
    NpcBounds getBounds() const { return bounds; }
    // Compare the stored NPC kind with a requested kind code.
    bool isKind(int value) const { return value==kind; }
};
// Tagged request ABI; the message type determines whether value is data or an address.
// type 6 QUERY_KIND: value=NpcKind; result=borrowed writable int*, set to 1 at
//                    first match, LEFT UNCHANGED when absent (caller initializes it).
// type 2 QUERY_BALL_BOUNDS: value=borrowed int* encoded as u32; first flag-bit-0
//                    creature containing the world-space ball writes 0, not 1.
//                    No match leaves it unchanged; result is unused in this case.
// types 0/1/3/4/8: value/result unused. 5/7/unknown types have no effect.
// No payload pointer is retained and no output is initialized on the caller's behalf.
// Effects: 0 updates active bit-0, non-disposable creatures; 1 commits the first
// bit-0 creature containing the ball; 3 restores non-bit-0 snapshots and disables
// bit-0 creatures; 4 snapshots/seeds path recording for non-bit-0 creatures;
// 8 commits every disposable creature. Queries stop at their first match.
struct NpcMessageRequest { char reserved00[4]; unsigned int type; unsigned int value; int* result; };
extern NpcPoint* GolfBall__Fv();
extern void Matrix_CopyAffine(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);
extern char npcResourceStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct Fn80197D30Iterator {
    NpcActorLink* volatile node;
    // End this non-owning actor-list iterator without touching its node.
    ~Fn80197D30Iterator() {}
};
// Initialize a borrowed iterator to the first node, or sentinel when empty.
inline void Fn80197D30Begin(Fn80197D30Iterator& p, NpcActorListHead* self) {
    NpcActorLink* node=self->first;
    Fn80197D30Iterator first={
        node
    };
    p.node=node;
}
struct Fn80197D30ArrayGuard {
    char* ptr;
    unsigned char* allocator;
    // Free exception-message storage unless its ownership has been transferred.
    ~Fn80197D30ArrayGuard() { if(ptr) __dla__FPv(ptr); }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control Fn80197D30Shared;
struct Fn80197D30Allocator {
    volatile unsigned char value;
    // Initialize the allocator tag used by the recovered vector/exception machinery.
    Fn80197D30Allocator() { value=0; }
    // End the allocator tag lifetime without releasing storage.
    ~Fn80197D30Allocator() {}
};
// Reduce the logical point count without freeing the allocation.
inline void Fn80197D30Clear(unsigned int& size, unsigned int count) {
    Fn80197D30Allocator allocator;
    size-=count;
}
namespace std {
struct exception {
    void* vtable;
    char* text;
    Fn80197D30Shared* owner;
    // Allocate/copy exception text and transfer it into a shared control block.
    exception(const char* const& s) {
        vtable=__vt__Q23std11logic_error;
        Fn80197D30Allocator baseAllocator;
        Fn80197D30Allocator allocator;
        unsigned int len=strlen(s)+1;
        text=(char*)__nwa__FUl(2*len-len);
        char* allocated=text;
        {
            Fn80197D30ArrayGuard guard={allocated,(unsigned char*)&allocator};
            owner=new Fn80197D30Shared(allocated);
            guard.ptr=0;
        }
        strcpy(text,s);
    }
    ~exception();
};
struct logic_error : exception {
    // Construct the logic-error base from borrowed message text.
    logic_error(const char* const& s) : exception(s) {}

    ~logic_error();
};
struct length_error : logic_error {
    // Construct the message base and install the length-error interface.
    length_error(const char* const& s) : logic_error(s) { vtable=__vt__Q23std12length_error; }
    ~length_error();
};
}
struct Fn80197D30Vector {
    Fn80197D30Vec* data;
    unsigned int size, capacity;
    // Start an empty temporary owning point vector.
    Fn80197D30Vector() : data(0),size(0),capacity(0) {}
    // Clear and free temporary point storage on normal exit or unwind.
    ~Fn80197D30Vector() {
        if(data) {
            Fn80197D30Clear(size,size);
            appFreeUni__FPv(data);
        }
    }
};
struct Fn80197D30Growth {
    Fn80197D30Vec* data;
    unsigned int size, capacity;
    unsigned int* oldCapacity;
    unsigned int prefix;
    // Start an empty growth guard borrowing the old vector capacity word.
    Fn80197D30Growth(unsigned int* p) : data(0),size(0),capacity(0),oldCapacity(p),prefix(0) {}
    // Release the temporary or displaced allocation after growth/unwind.
    ~Fn80197D30Growth() {
        if(data) {
            Fn80197D30Clear(size,size);
            appFreeUni__FPv(data);
        }
    }
};

#pragma pool_strings on
enum NpcMessage { kNpcMessageUpdate = 0, kNpcMessageCommitAtBall = 1, kNpcMessageQueryBallBounds = 2, kNpcMessageRestore = 3, kNpcMessageSave = 4, kNpcMessageUnused5 = 5, kNpcMessageQueryKind = 6, kNpcMessageUnused7 = 7, kNpcMessageCommitDisposable = 8 };
enum NpcPathCapacity { kInitialPathCapacity = 10, kMaximumPathCapacity = 0x15555555U, kSmallPathGrowthLimit = 0x071c71c7U, kMediumPathGrowthLimit = 0x0e38e38eU };
// Flag bit 0 separates message-triggered creatures from snapshot/path-recorded
// creatures; a more precise game-level name is unresolved.
enum NpcDispatchConstants { kNpcFlagBit0 = 1, kCourseSetupVtableSlot = 0x88 / 4,
    kSantaExtraSetupMode2 = 2, kSantaExtraSetupMode3 = 3, kSantaExtraSetupMode4 = 4,
    kCourseTaggedNpcObjectType = 10 };

// Dispatch NPC queries, activation, ball containment, path snapshot and restore messages across the actor list.
// Actors/links are retained by the circular list; this handler frees neither.
// SAVE must precede RESTORE for each non-bit-0 creature: restore reads pathPoints[0]
// without checking size. Bounds/target positions are in the ball's world space.
void CNPC_HandleMsg(NpcActorListHead* self, NpcMessageRequest* request) {
    unsigned int type = request->type;
    switch (type) {
        case kNpcMessageQueryKind: {
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node != sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item = static_cast<NpcCreatureView*>(iterator.node->object);
                if (item->isKind((int)request->value)) {
                    *request->result=1;
                    return;
                }
            }
            break;
        }
        case kNpcMessageUpdate: {
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item=static_cast<NpcCreatureView*>(iterator.node->object);
                if ((item->flags & kNpcFlagBit0) && item->kind!=kNpcDisposable && item->animationActive) {
                    item->update(0);
                }
            }
            break;
        }
        case kNpcMessageCommitAtBall: {
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item=static_cast<NpcCreatureView*>(iterator.node->object);
                if (item->flags & kNpcFlagBit0) {
                    NpcPoint* ballPosition=GolfBall__Fv();
                    NpcBounds bounds=static_cast<NpcCreatureView*>(iterator.node->object)->getBounds();
                    int inside=Fn80197D30Contains(bounds,ballPosition);
                    if (inside) {
                        item=static_cast<NpcCreatureView*>(iterator.node->object);
                        item->commit();
                        return;
                    }
                }
            }
            break;
        }
        case kNpcMessageQueryBallBounds: {
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                if (static_cast<NpcCreatureView*>(iterator.node->object)->flags & kNpcFlagBit0) {
                    NpcPoint* ballPosition=GolfBall__Fv();
                    NpcBounds bounds=static_cast<NpcCreatureView*>(iterator.node->object)->getBounds();
                    int inside=Fn80197D30Contains(bounds,ballPosition);
                    if (inside) {
                        *(int*)request->value=0;
                        return;
                    }
                }
            }
            break;
        }
        case kNpcMessageSave: {
            // Snapshot phase A: clear each non-bit-0 creature's recorded path and
            // reserve ten xyz entries. Temporary vectors own displaced allocations.
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item=static_cast<NpcCreatureView*>(iterator.node->object);
                if (!(item->flags & kNpcFlagBit0)) {
                    Fn80197D30Clear(item->pathPointCount,item->pathPointCount);
                    if (item->pathPointCapacity < kInitialPathCapacity) {
                        Fn80197D30Vector temp;
                        Fn80197D30Allocator allocator1;
                        temp.data=(Fn80197D30Vec*)appMallocDummy__FUl(kInitialPathCapacity * 12);
                        temp.capacity=kInitialPathCapacity;
                        Fn80197D30Vec* end=item->pathPoints+item->pathPointCount;
                        Fn80197D30Vec* src=item->pathPoints;
                        Fn80197D30Vec* dest=temp.data+temp.size;
                        Fn80197D30Allocator allocator2;
                        for (;src<end;++src,++dest) {
                            new(dest) Fn80197D30Vec(*src);
                            ++temp.size;
                        }
                        item->pathPointCount=0;
                        unsigned int oldCap=item->pathPointCapacity;
                        item->pathPointCapacity=temp.capacity;
                        Fn80197D30Vec* oldData=item->pathPoints;
                        Fn80197D30Allocator swapAllocator;
                        item->pathPoints=temp.data;
                        unsigned int oldSize=item->pathPointCount;
                        temp.capacity=oldCap;
                        temp.data=oldData;
                        item->pathPointCount=temp.size;
                        temp.size=oldSize;
                    }
                    // Snapshot phase B: seed the path with the current target. The
                    // expanded append/grow sequence below preserves unwind cleanup.
                    Fn80197D30Allocator allocator3;
                    if(item->pathPointCount<item->pathPointCapacity) {
                        Fn80197D30Vec* dest=item->pathPoints+item->pathPointCount;
                        new(dest) Fn80197D30Vec(item->targetPosition);
                        ++item->pathPointCount;
                    } else {
                        Fn80197D30Allocator allocator4;
                        Fn80197D30Allocator allocator5;
                        if(kMaximumPathCapacity-item->pathPointCapacity<1) {
                            throw std::length_error("vector length error");
                        }
                        unsigned int* capacity=&item->pathPointCapacity;
                        unsigned int needed=item->pathPointCount+1-*capacity;
                        Fn80197D30Growth temp(capacity);
                        unsigned int oldCapacity=item->pathPointCapacity;
                        if(needed>kMaximumPathCapacity-oldCapacity) {
                            throw std::length_error("vector length error");
                        }
                        unsigned int newCapacity;
                        if(oldCapacity<kSmallPathGrowthLimit) {
                            unsigned int grow=((oldCapacity+1)*3)/5;
                            newCapacity=oldCapacity+*(grow<needed ? &needed : &grow);
                        } else if(oldCapacity<kMediumPathGrowthLimit) {
                            unsigned int grow=(oldCapacity+1)/2;
                            newCapacity=oldCapacity+*(grow<needed ? &needed : &grow);
                        } else newCapacity=kMaximumPathCapacity;
                        Fn80197D30Allocator allocator6;
                        if(newCapacity>kMaximumPathCapacity) {
                            throw std::length_error("vector length error");
                        }
                        temp.data=(Fn80197D30Vec*)appMallocDummy__FUl(newCapacity*12);
                        temp.capacity=newCapacity;
                        temp.prefix=item->pathPointCount;
                        Fn80197D30Vec* dest=temp.data+item->pathPointCount+temp.size;
                        Fn80197D30Allocator allocator7;
                        for(unsigned int count=1;count!=0;--count,++dest) {
                            new(dest) Fn80197D30Vec(item->targetPosition);
                            ++temp.size;
                        }
                        Fn80197D30Vec* src=item->pathPoints+item->pathPointCount;
                        Fn80197D30Vec* backDest=temp.data+temp.prefix;
                        Fn80197D30Vec* start=item->pathPoints;
                        Fn80197D30Allocator allocator8;
                        while(src>start) {
                            --src;
                            --backDest;
                            new(backDest) Fn80197D30Vec(*src);
                            --temp.prefix;
                            ++temp.size;
                        }
                        item->pathPointCount=0;
                        unsigned int oldCap=item->pathPointCapacity;
                        item->pathPointCapacity=temp.capacity;
                        Fn80197D30Vec* oldData=item->pathPoints;
                        Fn80197D30Allocator swapAllocator;
                        item->pathPoints=temp.data;
                        temp.capacity=oldCap;
                        temp.data=oldData;
                        item->pathPointCount=temp.size;
                        temp.size=0;
                    }
                    // Snapshot phase C: recording now owns one initial target;
                    // retain animation, local-to-world transform, bank and active flag.
                    item->recordPath=1;
                    item->savedAnimationFrame=item->animationFrame;
                    Matrix_CopyAffine(item->savedMatrix,((CCreature*)item)->transform);
                    item->savedBankAngle=item->bankAngle;
                    item->savedAnimationActive=item->animationActive;
                }
            }
            break;
        }
        case kNpcMessageRestore: {
            // Bit-0 creatures are disabled; the others replay from the saved first
            // target, with cursor 1 denoting the NEXT point to consume.
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item=static_cast<NpcCreatureView*>(iterator.node->object);
                if(item->flags & kNpcFlagBit0) item->animationActive=0;
                else {
                    *(WVector*)(&item->targetPosition) = *(WVector*)item->pathPoints;
                    item->pathCursor=1;
                    item->recordPath=0;
                    item->animationFrame=item->savedAnimationFrame;
                    Matrix_CopyAffine(((CCreature*)item)->transform,item->savedMatrix);
                    item->bankAngle=item->savedBankAngle;
                    item->animationActive=item->savedAnimationActive;
                }
            }
            break;
        }
        case kNpcMessageCommitDisposable: {
            NpcActorLink* sentinel=(NpcActorLink*)&self->head;
            Fn80197D30Iterator iterator;
            Fn80197D30Begin(iterator,self);
            for (; iterator.node!=sentinel; iterator.node=iterator.node->next) {
                NpcCreatureView* item=static_cast<NpcCreatureView*>(iterator.node->object);
                if(item->kind==kNpcDisposable) item->commit();
            }
            break;
        }
        case kNpcMessageUnused5: case kNpcMessageUnused7:
        break;
        default:
        break;
    }
}

#pragma pool_strings reset

struct CreatureKindView {
    char reserved00[0x5C];
    unsigned int kind;
};

struct CreatureAnimationStateView {
    char reserved00[0xAD];
    unsigned char animationActive;
};

struct BorrowedPointerWord {
    unsigned int pointerBits;
};

struct CreatureSpawnDistanceView {
    char reserved00[0xB0];
    float ballSpawnOffsetDistance;
};

struct NpcUpdateLink {
    NpcUpdateLink* prev;
    NpcUpdateLink* next;
    void* object;
};

struct NpcUpdateList {
    char reserved00[0x1C];
    NpcUpdateLink sentinel;
};

struct SourceNameStringWords {
    unsigned int reserved00;
    unsigned int flags;
    unsigned int first;
    unsigned int second;
};

struct NameStringWords {
    unsigned int first;
    unsigned int second;
    unsigned int third;
};

struct NpcOwnedActorLink {
    // Links occupy the same offsets as NpcActorLink: preceding +0, following +4.
    NpcOwnedActorLink* prev;
    NpcOwnedActorLink* next;
    unsigned int value;
};

struct NpcOwnedActorList {
    int count;
    NpcOwnedActorLink sentinel;
};

struct NpcDefinitionActorLink {
    NpcDefinitionActorLink* prev;
    NpcDefinitionActorLink* next;
    void* object;
};

struct NpcDefinitionActorList {
    char reserved00[0x18];
    int count;
    NpcDefinitionActorLink sentinel;
};

struct CourseTaggedObject {
    char reserved00[0x10];
    int type;
    char placement[0x0C];
};

extern void appFreeUni__FPv(void*);
struct Fn801979C4Str {
    unsigned int flags;
    // NameStringWords overlays this word: heap length or bytes of the inline string.
    unsigned int lengthOrInlineBytes;
    char* ptr;
    // Free the copied course name only when its storage tag selects heap data.
    ~Fn801979C4Str() { if (flags >> 31) appFreeUni__FPv(ptr); }
    // Borrow the copied name bytes from inline or heap string storage.
    char* data() { return (flags >> 31) ? ptr : reinterpret_cast<char*>(this) + 1; }
};

struct NpcCourseRoot {
    char reserved00[0x8];
    void* courseData;
};

struct NpcCourseData {
    char reserved00[0x14];
    CourseTaggedObject* items;
    int itemCount;
    char reserved1C[0x60];
    int entryCount;
    char reserved80[0x1C];
    char* entryBase;
};

struct SharedNpcCourseView {
    char reserved00[0xB90];
    NpcCourseRoot* courseRoot;
};


extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, unsigned int, unsigned int, void*, void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void CNPC_CreateActorsFromDefinition(void*, void*, void*);
extern SharedNpcCourseView* g_sharedDoc;
extern char npcResourceStrings[];
extern float kFloat_32_Creature;
extern float kFloat_neg0_05_Creature;
extern double kDouble_SignedIntConversionBias_Creature;
extern float kFloat_0_Creature;
extern unsigned char SharedDoc_GetGolfRuleType();
extern void fn_80197D30ItemUpdate(NpcCreatureView*, int);
extern void fn_80197D30ItemCommit(NpcCreatureView*);
extern void* SharedDoc_GetCurrent();
extern void* GolfDoc_GetCurrent();
extern unsigned char CGolfDoc_GetCurrentHoleIndex(void*);
extern int String_ScanTokensAfterSubstring(const char*, const char*, ...);
extern void* CSanta_Ctor(void*, const char*);
extern void* CFighter_Ctor(void*, const char*);
extern void* CUfo_Ctor(void*, const char*);
extern void* ntPenguin_Ctor(void*, const char*);
extern void* ntNpc_Dragon_Ctor(void*, const char*, float);
extern void* ntNpc_PolaShip_Ctor(void*, const char*, float);
extern void* String_FormatTemporary(const char*, ...);
extern void* CDisposable_Ctor(void*, void*);
extern void CCreature_SetKind(CreatureKindView*, unsigned int);
extern void NpcActorList_PushBack(NpcOwnedActorList*, unsigned int*);

#pragma auto_inline off
// Store the creature kind used by NPC dispatch and kind queries.
void CCreature_SetKind(CreatureKindView* self, unsigned int value) {
    self->kind = value;
}
#pragma auto_inline on

#pragma auto_inline off
// Set whether the creature animation is active.
void CCreature_SetAnimationActive(CreatureAnimationStateView* self, unsigned char value) {
    self->animationActive = value;
}
#pragma auto_inline on

#pragma auto_inline off
// Store a borrowed pointer as a single word for the token parser.
void BorrowedNameRef_Set(BorrowedPointerWord* self, unsigned int value) {
    self->pointerBits = value;
}
#pragma auto_inline on

#pragma auto_inline off
// Set the creature's spawn offset distance relative to the ball.
void CCreature_SetBallSpawnOffsetDistance(CreatureSpawnDistanceView* self, float value) {
    self->ballSpawnOffsetDistance = value;
}
#pragma auto_inline on

struct Fn801987B8Obj124 {
    // Construct the Santa-family model wrapper from a borrowed resource name.
    Fn801987B8Obj124(const char* s) { CSanta_Ctor(this, s); }
    char storage[0x124];
};
struct Fn801987B8Obj124B {
    // Construct the fighter model wrapper from a borrowed resource name.
    Fn801987B8Obj124B(const char* s) { CFighter_Ctor(this, s); }
    char storage[0x124];
};
struct Fn801987B8Obj124C {
    // Construct the UFO model wrapper from a borrowed resource name.
    Fn801987B8Obj124C(const char* s) { CUfo_Ctor(this, s); }
    char storage[0x124];
};
struct Fn801987B8Obj124D {
    // Construct the penguin model wrapper from a borrowed resource name.
    Fn801987B8Obj124D(const char* s) { ntPenguin_Ctor(this, s); }
    char storage[0x124];
};
struct Fn801987B8Obj140 {
    // Construct the dragon wrapper with the parsed distance parameter.
    Fn801987B8Obj140(const char* s, float f) { ntNpc_Dragon_Ctor(this, s, f); }
    char storage[0x140];
};
struct Fn801987B8Obj150 {
    // Construct the ship wrapper with the parsed distance parameter.
    Fn801987B8Obj150(const char* s, float f) { ntNpc_PolaShip_Ctor(this, s, f); }
    char storage[0x150];
};
struct Fn801987B8Obj134 {
    // Format the npc_ tag suffix as a .pet name and construct the disposable model.
    Fn801987B8Obj134(const void* found) {
        void* s = String_FormatTemporary(((NpcStringPool*)npcResourceStrings)->petNameFormat, static_cast<const char*>(found) + 4);
        CDisposable_Ctor(this, s);
    }
    char storage[0x134];
};

#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Recognize course-name tags, create the corresponding creature and register it with this NPC actor.
// nameArg is a borrowed NUL-terminated string, layout a borrowed placement block.
// Tags are tested in order (santa, flight, ufo, penguinwalk, drg, ship, npc_);
// only the first matching branch runs. A rejected santa gate does not fall through.
// Santa extras require rule type 0, gameMode 2/3/4 and holeCode%100 < 3; meanings
// of those numeric modes are unresolved here. The extras use three Mozzi models
// plus Santa, with setup offsets -0.05*[0,1,2,4] (the gap at 3 is intentional).
// Slot +0x88 receives (object,placement,offset,0); offset units are unresolved.
// After setup, kind is assigned and the actor list retains the new creature.
void CNPC_CreateObjectsFromCourseName(void* self, void* layout, void* nameArg) {
    void* object = 0;
    const char* name = static_cast<const char*>(nameArg);
    if (strstr(name, ((NpcStringPool*)npcResourceStrings)->santaTag)) {
        if (!SharedDoc_GetGolfRuleType()) {
            unsigned int gameMode = ((CSharedDoc*)SharedDoc_GetCurrent())->gameMode;
            // 0x801987B8: modes 2, 3 and 4 share the extra NPC setup.
            if (gameMode == kSantaExtraSetupMode2 ||
            (gameMode = ((CSharedDoc*)SharedDoc_GetCurrent())->gameMode) == kSantaExtraSetupMode3 ||
            (gameMode = ((CSharedDoc*)SharedDoc_GetCurrent())->gameMode) == kSantaExtraSetupMode4) {
                void* extraReceiver0 = (void*)GolfDoc_GetCurrent();
                unsigned int holeCode = ((SharedNpcHoleCodes*)SharedDoc_GetCurrent())->holeCodes[(CGolfDoc_GetCurrentHoleIndex(extraReceiver0)) & 0xFF];
                unsigned int courseVariant = holeCode - ((holeCode / 100) * 100);
                if (courseVariant < 3) {
                    unsigned int key;
                    float spawnDistance = kFloat_32_Creature;
                    BorrowedNameRef_Set(reinterpret_cast<BorrowedPointerWord*>(&key), reinterpret_cast<unsigned int>(name));
                    if (!String_ScanTokensAfterSubstring(reinterpret_cast<const char*>(&key), ((NpcStringPool*)npcResourceStrings)->radiusToken, ((NpcStringPool*)npcResourceStrings)->floatFormat, &spawnDistance)) spawnDistance = kFloat_32_Creature;
                    float spawnSpacing = kFloat_neg0_05_Creature;
                    int i;
                    for (i = 0; 3 > i; ++i) {
                        object = new Fn801987B8Obj124(((NpcStringPool*)npcResourceStrings)->mozziModel);
                        CCreature_SetBallSpawnOffsetDistance(reinterpret_cast<CreatureSpawnDistanceView*>(object), spawnDistance);
                        typedef void (*Set)(void*, void*, float, int);
                        reinterpret_cast<Set>( (*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot] )(object, layout, spawnSpacing * (float)i, 0);
                        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcSanta);
                        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
                    }
                    object = new Fn801987B8Obj124(((NpcStringPool*)npcResourceStrings)->santaModel);
                    CCreature_SetBallSpawnOffsetDistance(reinterpret_cast<CreatureSpawnDistanceView*>(object), spawnDistance);
                    typedef void (*Set)(void*, void*, float, int);
                    reinterpret_cast<Set>( (*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot] )(object, layout, spawnSpacing * (float)(i + 1), 0);
                    CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcSanta);
                    NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
                }
            }
        }
    } else if (strstr(name, ((NpcStringPool*)npcResourceStrings)->flightTag)) {
        object = new Fn801987B8Obj124B(((NpcStringPool*)npcResourceStrings)->fighterModel);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcFighter);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    } else if (strstr(name, ((NpcStringPool*)npcResourceStrings)->ufoTag)) {
        object = new Fn801987B8Obj124C(((NpcStringPool*)npcResourceStrings)->ufoModel);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcUfo);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    } else if (strstr(name, ((NpcStringPool*)npcResourceStrings)->penguinWalkTag)) {
        object = new Fn801987B8Obj124D(((NpcStringPool*)npcResourceStrings)->penguinModel);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcPenguin);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    } else if (strstr(name, ((NpcStringPool*)npcResourceStrings)->dragonTag)) {
        float dragonDistance = 0.0f;
        unsigned int key2;
        BorrowedNameRef_Set(reinterpret_cast<BorrowedPointerWord*>(&key2), reinterpret_cast<unsigned int>(name));
        if (!String_ScanTokensAfterSubstring(reinterpret_cast<const char*>(&key2), ((NpcStringPool*)npcResourceStrings)->distanceToken, ((NpcStringPool*)npcResourceStrings)->floatFormat, &dragonDistance)) dragonDistance = 0.0f;
        object = new Fn801987B8Obj140(((NpcStringPool*)npcResourceStrings)->dragonModel, dragonDistance);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcDragon);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    } else if (strstr(name, ((NpcStringPool*)npcResourceStrings)->shipTag)) {
        float shipDistance = 0.0f;
        unsigned int key3;
        BorrowedNameRef_Set(reinterpret_cast<BorrowedPointerWord*>(&key3), reinterpret_cast<unsigned int>(name));
        if (!String_ScanTokensAfterSubstring(reinterpret_cast<const char*>(&key3), ((NpcStringPool*)npcResourceStrings)->distanceToken, ((NpcStringPool*)npcResourceStrings)->floatFormat, &shipDistance)) shipDistance = 0.0f;
        object = new Fn801987B8Obj150(((NpcStringPool*)npcResourceStrings)->shipModel, shipDistance);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcShip);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    } else if (void* found = (void*)strstr(name, ((NpcStringPool*)npcResourceStrings)->npcTag)) {
        object = new Fn801987B8Obj134(found);
        typedef void (*Set)(void*, void*, float, int);
        reinterpret_cast<Set>((*reinterpret_cast<void***>(object))[kCourseSetupVtableSlot])(object, layout, 0.0f, 0);
        CCreature_SetKind(reinterpret_cast<CreatureKindView*>(object), kNpcDisposable);
        NpcActorList_PushBack(reinterpret_cast<NpcOwnedActorList*>(((NpcActorListView*)self)->actors), reinterpret_cast<unsigned int*>(&object));
    }
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

class V80 { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual void v74(); virtual void v78(); virtual void v7C(); virtual void v80(float);
};
// Advance every registered creature by the supplied frame delta.
void CNPC_OnProcess(NpcUpdateList* self, float value) {
    NpcUpdateLink* link = self->sentinel.next;
    NpcUpdateLink* sentinel = &self->sentinel;
    while (link != sentinel) {
        ((V80*)link->object)->v80(value);
        link = link->next;
    }
}

#pragma auto_inline off
// Copy an inline string directly or allocate and copy a heap-backed string with exception-safe ownership.
void MWString_CopyConstructForNameLookup(NameStringWords* self, const SourceNameStringWords* source) {
    if ((source->flags >> 31) == 0) {
        self->first = source->flags;
        self->second = source->first;
        self->third = source->second;
    } else {
        self->first = 0;
        self->second = 0;
        self->third = 0;
        String_ReserveCapacity(self, source->first);
        struct Tag {
            unsigned char byte;
            char pad[3];
        };
        volatile Tag tag;
        Tag value;
        value.byte = tag.byte;
        String_ReplaceRange(self, 0, 0, reinterpret_cast<void*>(source->second), reinterpret_cast<void*>(source->second + source->first), &value);
    }
}

#pragma auto_inline on

struct Fn80197C08Guard {
    char allocator;
    NpcOwnedActorLink* sentinel;
    NpcOwnedActorLink* node;
    // Guard a newly allocated actor-list node until it is linked.
    Fn80197C08Guard(NpcOwnedActorLink* s, NpcOwnedActorLink* n) : sentinel(s), node(n) {}
    // Free an uncommitted node, leaving committed list nodes untouched.
    ~Fn80197C08Guard() { if (node) appFreeUni__FPv(node); }
};
struct Fn80197C08Iterator {
    NpcOwnedActorLink* node;
    // End the non-owning node iterator without freeing its node.
    ~Fn80197C08Iterator() {}
};
// Copy one actor pointer word with the original exception boundary.
inline void Fn80197C08Construct(unsigned int* p, unsigned int v) {
    try {
        *p = v;
    } catch (...) {
        throw;
    }
}
struct Fn80197C08Value {
    unsigned int value;
    // Construct the list payload from the borrowed actor pointer word.
    Fn80197C08Value(const unsigned int& v) { Fn80197C08Construct(&value, v); }
};

// Allocate and link one actor before the sentinel (the back of the circular NPC list).
// Copies the pointer word from value (not the pointed-to creature); list owns the
// allocated link. Creature destruction is performed separately by CNPC_OnInit.
void NpcActorList_PushBack(NpcOwnedActorList* self, unsigned int* value) {
    NpcOwnedActorLink* sentinel = &self->sentinel;
    volatile unsigned char allocator = 0;
    NpcOwnedActorLink* link = static_cast<NpcOwnedActorLink*>(appMallocDummy__FUl(12));
    Fn80197C08Iterator allocated;
    Fn80197C08Iterator position;
    position.node = link;
    allocated.node = link;
    Fn80197C08Guard guard(sentinel, link);
    new (&link->value) Fn80197C08Value(*value);
    guard.node = 0;
    sentinel->prev->next = link;
    link->prev = sentinel->prev;
    sentinel->prev = link;
    link->next = sentinel;
    ++self->count;
}

#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Destroy existing creatures, then rebuild the actor list from course definitions and tagged course objects.
// Phase order: destroy actor payloads -> unlink/free list nodes -> parse definition
// entries -> parse type-10 course tags. The sentinel remains embedded in self.
void CNPC_OnInit(NpcDefinitionActorList* self) {
    NpcDefinitionActorLink* link = self->sentinel.next;
    NpcDefinitionActorLink* sentinel = &self->sentinel;
    for (; link != sentinel; link = link->next) {
        void* object = link->object;
        if (object != 0) {
            if (object != 0) {
                void** vtable = *static_cast<void***>(object);
                reinterpret_cast<void (*)(void*, int)>(vtable[31])(object, 1);
            }
            link->object = 0;
            object = 0;
        }
    }
    // Phase 2: payloads are destroyed; unlink and release only the list nodes.
    NpcDefinitionActorLink* head = &self->sentinel;
    NpcDefinitionActorLink* first = self->sentinel.next;
    if (first != head) {
        NpcDefinitionActorLink* tail = head->prev;
        first->prev->next = tail->next;
        tail->next->prev = first->prev;
        while (first != head) {
            NpcDefinitionActorLink* cur = first;
            first = first->next;
            appFreeUni__FPv(cur);
            --self->count;
        }
    }
    // Phase 3: rebuild definition actors first, followed by tagged course placements.
    NpcCourseData* data;
    int off = 0;
    for (int i = 0; i < (data = static_cast<NpcCourseData*>(g_sharedDoc->courseRoot->courseData))->entryCount; off += 0x5C, ++i) {
        char* record = data->entryBase + off;
        if (static_cast<unsigned char>(record[0]) == 1)
        CNPC_CreateActorsFromDefinition(self, record + 0x44, record + 1);
    }
    for (CourseTaggedObject* item = data->items; item != static_cast<NpcCourseData*>(g_sharedDoc->courseRoot->courseData)->items + static_cast<NpcCourseData*>(g_sharedDoc->courseRoot->courseData)->itemCount; ++item) {
        if (item->type == kCourseTaggedNpcObjectType) {
            Fn801979C4Str str;
            MWString_CopyConstructForNameLookup(reinterpret_cast<NameStringWords*>(&str), reinterpret_cast<SourceNameStringWords*>(item));
            char* dataPtr = str.data();
            CNPC_CreateObjectsFromCourseName(self, item->placement, dataPtr);
        }
    }
}


} // extern "C"

// Private views must retain the original 32-bit ABI offsets.
typedef char u801979C4_layout_check_0[(NPC_OFFSET(NpcStringPool, defaultAnimation) == 0x11E) ? 1 : -1];
typedef char u801979C4_layout_check_1[(NPC_OFFSET(NpcStringPool, opacityKeyFormat) == 0x12A) ? 1 : -1];
