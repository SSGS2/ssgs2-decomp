// The shared document singleton (CSharedDoc): course and round settings, player records, the seven
// owned lists and two trees, and the bonus and item-selection inventory.

#include "MSL/list.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
// Seven owned lists share a count and an embedded two-link sentinel.
// Header offsets: count +0, embedded sentinel +4 (previous), +8 (next).
// Nodes begin with those two links; this unit frees the node allocation, with no
// separate payload destructor. No proof here that a payload pointer would be owned.
// Seven specializations occupy 0x7A4, 0x7B0, 0x7BC, 0x7C8, 0x7D4, 0x7E0, 0x7EC.
// Their gameplay payload roles are unresolved; use these stable offsets to distinguish
// them. The adjacent 0x7F8 bonus-item vector is a separate allocation, not list 7.
struct SharedOwnedList {
    int count;
    void* sentinelPrevious;
    void* sentinelNext;
};
struct SharedDocOwnedLists {
    char padding000[0x7a4];
    SharedOwnedList lists[7];
};
// The constructor initializes four equal player records before the mode descriptors.
struct SharedDocPlayerStorage {
    char documentPrefix[0x10];
    char players[4][0x1D4];
};
struct SharedPlayerRuntimeStorage {
    char playerPrefix[0x180];
    unsigned int runtimeWords[5];
};
typedef char SharedPlayerRuntimeOffset[((int)&((SharedPlayerRuntimeStorage*)0)->runtimeWords == 0x180) ? 1 : -1];
typedef char SharedOwnedList_size[(sizeof(SharedOwnedList) == 12) ? 1 : -1];

enum {
    kFullRoundHoles = 18,
    kDefaultTurnTimeMs = 40000,
    kAuxiliaryItemSlots = 5
};
enum AuxPartModifier {
    kAuxAdditiveProperty = 0,
    kAuxRateProperty1 = 1,
    kAuxRateProperty2 = 2,
    kAuxRateProperty3 = 3
};
// Tree/list link views only; payloads and ownership remain with the original containers.
struct SharedTreeLinks {
    void* left;
    void* right;
};
struct SharedTreeStorage {
    unsigned int count;
    void* root;
    void* tail;
    void* sentinel;
};
struct SharedListLinks {
    void* previous;
    void* next;
};

#include "iff/IffStruct.h"
#include "classes/Record801F31C0.h"
#define AsRecord801F31C0(p) ((CRoundCourseSettings*)(p))
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDocSingleton.h"

#define AsCSharedDoc(p) ((CSharedDoc*)(p))

extern "C" {

void ItemSelection_SetInventory(void*);
void Tree_DestroyNodesAlt(void*, void*);
void Tree_DestroyNodes(void*, void*);
void SharedDoc_SelectCourse(int);
void appFreeUni__FPv(void*);
int ItemInventory_IsSlot14ClubSetId(void*);
int ItemInventory_FindAuxPartData(void*, void*);
char* strcpy(char*, const char*);
void* BaseObject_ctor(void*);
void BaseObject_dtor(void*, int);
void __dl__FPv(void*);
void* memset(void*, int, unsigned int);
extern float kAuxPartDefaultAdditiveModifier;
extern float kAuxPartDefaultRateModifier;
extern const float kAuxPartPropertyRatePercentDivisor;

extern char __vt__10CSharedDoc[];
extern char s_sharedDocSingletonVtable[];
extern void* g_sharedDoc;

// Initialize course-selection settings to an empty course and a full 18-hole round.
void* RoundCourseSettings_Init(void* self) {
    char* bytes = (char*)self;
    int zero = 0;
    AsRecord801F31C0(bytes)->courseId = 0;
    AsRecord801F31C0(bytes)->selectedCourseData = zero;
    AsRecord801F31C0(bytes)->golfRuleType = (char)zero;
    AsRecord801F31C0(bytes)->roundHoleCount = kFullRoundHoles;
    AsRecord801F31C0(bytes)->unk_10E = (char)zero;
    AsRecord801F31C0(bytes)->turnTimeLimitMs = zero;
    AsRecord801F31C0(bytes)->unk_114 = zero;
    memset(bytes + 9, 0, 0x104);
    return self;
}

// Construct the shared-document singleton and publish its address.
CSharedDocSingleton::CSharedDocSingleton() {
    BaseObject_ctor(this);
    *(void**)this = (void*)s_sharedDocSingletonVtable;
    g_sharedDoc = this;
}

// Clear the shared-document singleton and destroy its base, optionally freeing storage.
void* SharedDoc_dtor(void* self, int flags) {
    if (self != 0) {
        g_sharedDoc = 0;
        BaseObject_dtor(self, 0);
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// These three macros unroll alternate-tree destruction before the recursive library call.
// Inline-function forms change register allocation; keep the scopes and declaration order.
#define FREE_LEAF(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* v0 = *(void**)(n);                 \
            if (v0 != 0)                             \
                Tree_DestroyNodesAlt(self, v0);      \
            void* v1 = ((SharedTreeLinks*)n)->right; \
            if (v1 != 0)                             \
                Tree_DestroyNodesAlt(self, v1);      \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)

#define FREE_PAIR(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* c0 = *(void**)(n);                 \
            FREE_LEAF(c0);                           \
            void* c1 = ((SharedTreeLinks*)n)->right; \
            FREE_LEAF(c1);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)

#define FREE_TOP(n)                                                                  \
    do {                                                                             \
        if ((n) != 0) {                                                              \
            void* t0 = *(void**)(n); /* distinct names avoid capture by FREE_PAIR */ \
            FREE_PAIR(t0);                                                           \
            void* t1 = ((SharedTreeLinks*)n)->right;                                 \
            FREE_PAIR(t1);                                                           \
            appFreeUni__FPv(n);                                                      \
        }                                                                            \
    } while (0)

#pragma push
#pragma use_lmw_stmw on
// Destroy both branches of the alternate tree and optionally free its container.
void* SharedTreeStorage_dtor(void* self, int flags) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            void* root = ((SharedTreeStorage*)self)->root;
                            if (root != 0) {
                                void* n0 = *(void**)root;
                                FREE_TOP(n0);
                                void* n1 = ((SharedTreeLinks*)root)->right;
                                FREE_TOP(n1);
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Destroy both branches of the tree and optionally free its container.
void* SharedTree_dtor(void* self, int flags) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            void* root = ((SharedTreeStorage*)self)->root;
                            if (root != 0) {
                                void* n0 = *(void**)root;
#define DT_LEAF(n)                                   \
    do {                                             \
        if (n) {                                     \
            void* v0 = *(void**)n;                   \
            if (v0)                                  \
                Tree_DestroyNodes(self, v0);         \
            void* v1 = ((SharedTreeLinks*)n)->right; \
            if (v1)                                  \
                Tree_DestroyNodes(self, v1);         \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define DT_PAIR(n)                                      \
    do {                                                \
        if (n) {                                        \
            void* leaf0 = *(void**)n;                   \
            DT_LEAF(leaf0);                             \
            void* leaf1 = ((SharedTreeLinks*)n)->right; \
            DT_LEAF(leaf1);                             \
            appFreeUni__FPv(n);                         \
        }                                               \
    } while (0)
#define DT_TOP(n)                                       \
    do {                                                \
        if (n) {                                        \
            void* pair0 = *(void**)n;                   \
            DT_PAIR(pair0);                             \
            void* pair1 = ((SharedTreeLinks*)n)->right; \
            DT_PAIR(pair1);                             \
            appFreeUni__FPv(n);                         \
        }                                               \
    } while (0)
                                DT_TOP(n0);
                                void* n1 = ((SharedTreeLinks*)root)->right;
                                DT_TOP(n1);
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
#undef DT_TOP
#undef DT_PAIR
#undef DT_LEAF
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

#undef FREE_TOP
#pragma pop
#undef FREE_PAIR
#undef FREE_LEAF

// Initialize an empty tree with its sentinel at the root-link word.
void TreeHeader_InitSentinel(void* self) {
    int* fields = (int*)self;
    fields[0] = 0;
    fields[1] = 0;
    fields[3] = (int)((char*)self + 4);
}

// Initialize an empty doubly linked list with both links pointing to its sentinel.
void ListHeader_InitSentinel(void* self) {
    int* fields = (int*)self;
    fields[0] = 0;
    fields[2] = (int)((char*)self + 4);
    fields[1] = (int)((char*)self + 4);
}

// Clear the player's five runtime words without disturbing its equipment and round data.
void* SharedPlayer_ClearRuntimeWords(void* self) {
    memset(((SharedPlayerRuntimeStorage*)self)->runtimeWords, 0,
           sizeof(((SharedPlayerRuntimeStorage*)0)->runtimeWords));
    return self;
}

// Bind item selection to this shared document inventory subobject.
void CSharedDoc::BindItemSelectionInventory() {
    ItemSelection_SetInventory(itemInventory);
}

void __dt__12CItemManagerFv(void*, int);

#define FREE_44_1(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* v0 = *(void**)(n);                 \
            if (v0 != 0)                             \
                Tree_DestroyNodes(base + 0x794, v0); \
            void* v1 = ((SharedTreeLinks*)n)->right; \
            if (v1 != 0)                             \
                Tree_DestroyNodes(base + 0x794, v1); \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define FREE_44_2(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a2 = *(void**)(n);                 \
            FREE_44_1(a2);                           \
            void* b2 = ((SharedTreeLinks*)n)->right; \
            FREE_44_1(b2);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define FREE_44_3(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a3 = *(void**)(n);                 \
            FREE_44_2(a3);                           \
            void* b3 = ((SharedTreeLinks*)n)->right; \
            FREE_44_2(b3);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define FREE_44_4(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a4 = *(void**)(n);                 \
            FREE_44_3(a4);                           \
            void* b4 = ((SharedTreeLinks*)n)->right; \
            FREE_44_3(b4);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
// Release bonus items, seven lists and two trees, then clear the shared-document singleton.
CSharedDoc::~CSharedDoc() {
    char* base = (char*)this;
    *(void**)this = (void*)__vt__10CSharedDoc;
    // Phase 1: invalidate player records, destroy the inventory, release bonus IDs.
    memset(((SharedDocPlayerStorage*)this)->players, 0, sizeof(((SharedDocPlayerStorage*)0)->players));
    __dt__12CItemManagerFv(((char*)&itemInventory), -1);
    // 0x7F8 single node
    if (base + 0x7F8)
        if (base + 0x7F8)
            if (base + 0x7F8)
                if (base + 0x7F8)
                    if (base + 0x7F8) {
                        void* n = AsCSharedDoc(base)->bonusItemIds;
                        if (n != 0) {
                            (*(int*)&AsCSharedDoc(base)->bonusItemCount) -= AsCSharedDoc(base)->bonusItemCount;
                            appFreeUni__FPv(n);
                        }
                    }
    // Plain C for each of the seven blocks below (lists 6..0; list i has count at 0x7A4+12*i, sentinel at count+4):
    //   for (node = list.first; node != &list.sentinel; node = next) { free(node); list.count--; }
    // the pointer juggling with p/q first splices the whole range out of the sentinel; the stacked
    // `if (base + 0xNNN)` lines are the same always-true test repeated by inlined destructors (kept to match).
    // Link naming: the sentinel sits at list+4 with its previous link first and next link second, so
    // sentinelNext (list+8) is the first node and node->next is a node's second word; the naming is consistent.
    // Phase 2: drain lists 6..0 in reverse member order. Each block detaches the
    // range, advances before freeing, and decrements its own count to zero.
    // List 6: sentinel 0x7F0, count 0x7EC; payload role unresolved.
    if (base + 0x7EC)
        if (base + 0x7EC)
            if (base + 0x7EC)
                if (base + 0x7EC) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[6].sentinelNext;
                    void* sentinel = base + 0x7F0;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[6].count));
                        }
                    }
                }
    // List 5: sentinel 0x7E4, count 0x7E0; payload role unresolved.
    if (base + 0x7E0)
        if (base + 0x7E0)
            if (base + 0x7E0)
                if (base + 0x7E0) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[5].sentinelNext;
                    void* sentinel = base + 0x7E4;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[5].count));
                        }
                    }
                }
    // List 4: sentinel 0x7D8, count 0x7D4; payload role unresolved.
    if (base + 0x7D4)
        if (base + 0x7D4)
            if (base + 0x7D4)
                if (base + 0x7D4) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[4].sentinelNext;
                    void* sentinel = base + 0x7D8;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[4].count));
                        }
                    }
                }
    // List 3: sentinel 0x7CC, count 0x7C8; payload role unresolved.
    if (base + 0x7C8)
        if (base + 0x7C8)
            if (base + 0x7C8)
                if (base + 0x7C8) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[3].sentinelNext;
                    void* sentinel = base + 0x7CC;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[3].count));
                        }
                    }
                }
    // List 2: sentinel 0x7C0, count 0x7BC; payload role unresolved.
    if (base + 0x7BC)
        if (base + 0x7BC)
            if (base + 0x7BC)
                if (base + 0x7BC) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[2].sentinelNext;
                    void* sentinel = base + 0x7C0;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[2].count));
                        }
                    }
                }
    // List 1: sentinel 0x7B4, count 0x7B0; payload role unresolved.
    if (base + 0x7B0)
        if (base + 0x7B0)
            if (base + 0x7B0)
                if (base + 0x7B0) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[1].sentinelNext;
                    void* sentinel = base + 0x7B4;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[1].count));
                        }
                    }
                }
    // List 0: sentinel 0x7A8, count 0x7A4; payload role unresolved.
    if (base + 0x7A4)
        if (base + 0x7A4)
            if (base + 0x7A4)
                if (base + 0x7A4) {
                    void* node = ((SharedDocOwnedLists*)base)->lists[0].sentinelNext;
                    void* sentinel = base + 0x7A8;
                    if (node != sentinel) {
                        void** p = *(void***)sentinel;
                        void** q = *(void***)node;
                        q[1] = p[1];
                        *(void**)p[1] = *(void**)node;
                        while (node != sentinel) {
                            void* current = node;
                            node = ((SharedListLinks*)node)->next;
                            appFreeUni__FPv(current);
                            --*(int*)(base + IFF_OFFSET(SharedDocOwnedLists, lists[0].count));
                        }
                    }
                }
    // Phase 3: free both tree node sets, then clear the singleton and destroy its base.
    if (base + 0x794)
        if (base + 0x794)
            if (base + 0x794)
                if (base + 0x794)
                    if (base + 0x794)
                        if (base + 0x794) {
                            void* root44 = AsCSharedDoc(base)->secondOwnedTreeRoot;
                            FREE_44_4(root44);
                        }
#undef FREE_44_4
#undef FREE_44_3
#undef FREE_44_2
#undef FREE_44_1
#define FREE_47_1(n)                                    \
    do {                                                \
        if ((n) != 0) {                                 \
            void* v0 = *(void**)(n);                    \
            if (v0 != 0)                                \
                Tree_DestroyNodesAlt(base + 0x784, v0); \
            void* v1 = ((SharedTreeLinks*)n)->right;    \
            if (v1 != 0)                                \
                Tree_DestroyNodesAlt(base + 0x784, v1); \
            appFreeUni__FPv(n);                         \
        }                                               \
    } while (0)
#define FREE_47_2(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a2 = *(void**)(n);                 \
            FREE_47_1(a2);                           \
            void* b2 = ((SharedTreeLinks*)n)->right; \
            FREE_47_1(b2);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define FREE_47_3(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a3 = *(void**)(n);                 \
            FREE_47_2(a3);                           \
            void* b3 = ((SharedTreeLinks*)n)->right; \
            FREE_47_2(b3);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
#define FREE_47_4(n)                                 \
    do {                                             \
        if ((n) != 0) {                              \
            void* a4 = *(void**)(n);                 \
            FREE_47_3(a4);                           \
            void* b4 = ((SharedTreeLinks*)n)->right; \
            FREE_47_3(b4);                           \
            appFreeUni__FPv(n);                      \
        }                                            \
    } while (0)
    if (base + 0x784)
        if (base + 0x784)
            if (base + 0x784)
                if (base + 0x784)
                    if (base + 0x784)
                        if (base + 0x784) {
                            void* root47 = AsCSharedDoc(base)->firstOwnedTreeRoot;
                            FREE_47_4(root47);
                        }
#undef FREE_47_4
#undef FREE_47_3
#undef FREE_47_2
#undef FREE_47_1
    if (this != 0) {
        g_sharedDoc = 0;
        BaseObject_dtor(this, 0);
    }
}

// Initialize an empty pointer/size/capacity storage triple.
void PointerVector_InitEmptyStorage(void* self) {
    int* fields = (int*)self;
    fields[0] = 0;
    fields[1] = 0;
    fields[2] = 0;
}

// Initialize an empty tree header and its embedded sentinel link.
void TreeHeader_InitEmptyWithSentinel(void* self) {
    int* fields = (int*)self;
    fields[0] = 0;
    fields[1] = 0;
    ((SharedTreeStorage*)self)->sentinel = (char*)self + 4;
}

// Select course zero, reset round flags and use 18 holes with a 40-second turn limit.
void SharedDoc_InitDefaultCourseSettings() {
    SharedDoc_SelectCourse(0);
    AsCSharedDoc(g_sharedDoc)->golfRuleType = 0;
    {
        char* g = (char*)g_sharedDoc;
        strcpy(g + 0x8C1, (char*)AsCSharedDoc(g)->strokePlayName);
    }
    AsCSharedDoc(g_sharedDoc)->roundHoleOrder = 0;
    AsCSharedDoc(g_sharedDoc)->roundHoleCount = kFullRoundHoles;
    ((char*)g_sharedDoc)[0x9C6] = 0;
    (*(int*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs) = kDefaultTurnTimeMs;
    AsCSharedDoc(g_sharedDoc)->unk_9CC = 0;
}

// Unlink and free all list nodes, then optionally free the list container.
void* dtor_801F38CC(MSLPointerList* self, int flags) {
    return ::std::__list_deleting_dtor(self, flags);
}

// Combine five equipped auxiliary items: add property 0 bonuses or multiply nonzero property 1..3 percentages.
// self is expected to be g_sharedDoc: only the initial golfDoc guard reads self;
// all player records and item lookups use the global. A nonnull self->golfDoc enables
// the global playerCount check. Without it, caller must supply a valid playerIndex.
// Player p begins at global +0x10+p*0x1D4; five equipment pointers are at player+0x180,
// and the inventory pointer is player+0x1BC. All pointers are borrowed.
// Property 0 returns raw additive item units; 1..3 return a dimensionless product
// of nonzero percentages (100 is neutral, 0 is skipped). Gameplay labels for the
// first two rate properties are unresolved; property 3 scales the Pang reward sum
// in CBonusInfo_UpdatePangRewards. Unsupported properties return the rate default.
float CSharedDoc_ComputeAuxPartModifier(void* self, unsigned int playerIndex, int property) {
    if ((*(int*)&AsCSharedDoc(self)->golfDoc) != 0) {
        char* sharedDocBytes = (char*)*(void* volatile*)&g_sharedDoc;
        void* golfDoc = (void*)AsCSharedDoc(sharedDocBytes)->golfDoc;
        if (((CGolfDoc*)golfDoc)->playerCount <= playerIndex) {
            if (property == kAuxAdditiveProperty)
                return kAuxPartDefaultAdditiveModifier;
            return kAuxPartDefaultRateModifier;
        }
        void* playerInventory = *(void**)(sharedDocBytes + playerIndex * 0x1D4 + 0x1CC);
        if (ItemInventory_IsSlot14ClubSetId(playerInventory) != 0 && property == kAuxAdditiveProperty)
            return kAuxPartDefaultAdditiveModifier;
    }
    char* entryBase = (char*)*(void* volatile*)&g_sharedDoc + playerIndex * 0x1D4;
    char* auxiliarySlots = entryBase + 0x190;
    float result = property == kAuxAdditiveProperty ? kAuxPartDefaultAdditiveModifier : kAuxPartDefaultRateModifier;
    for (int i = 0; i < kAuxiliaryItemSlots; ++i, auxiliarySlots += 4) {
        void* equippedItem = *(void**)auxiliarySlots;
        if (equippedItem == 0)
            continue;
        IFF_STRUCT::sAuxPart* auxPart = (IFF_STRUCT::sAuxPart*)ItemInventory_FindAuxPartData(
            AsCSharedDoc(*(void* volatile*)&g_sharedDoc)->itemInventory, equippedItem);
        if (auxPart == 0)
            continue;
        switch (property) {
        case kAuxAdditiveProperty: {
            int modifierValue = auxPart->property0Bonus;
            result += (float)modifierValue;
            break;
        }
        case kAuxRateProperty1: {
            unsigned short modifierValue = auxPart->propertyRates[0];
            if (modifierValue != 0)
                result *= ((float)modifierValue / kAuxPartPropertyRatePercentDivisor);
            break;
        }
        case kAuxRateProperty2: {
            unsigned short modifierValue = auxPart->propertyRates[1];
            if (modifierValue != 0)
                result *= ((float)modifierValue / kAuxPartPropertyRatePercentDivisor);
            break;
        }
        case kAuxRateProperty3: {
            unsigned short modifierValue = auxPart->propertyRates[2];
            if (modifierValue != 0)
                result *= ((float)modifierValue / kAuxPartPropertyRatePercentDivisor);
            break;
        }
        }
    }
    return result;
}

} // extern "C"
