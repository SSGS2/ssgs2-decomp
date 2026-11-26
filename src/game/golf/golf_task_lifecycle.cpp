// The golf task's lifetime: allocation and construction, the actor table it
// builds from the story and party modes, and teardown.
//
// Loading and drawing, the grid, and the interface thunks live in the
// neighboring golf task units.

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CGolfTask.h"
#include "classes/CTask.h"

#define AsCGolfTask(p) ((CGolfTask*)(p))

extern "C" {

void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
void* memset(void*, int, unsigned long);
void appFreeUni__FPv(void*);
void InputManager_Init(void*);
void InputManager_SetCursorDrawOptions(void*, int, int, int, int);
void InputManager_SetCursorColorOverride(void*, int, int, int, int, int);
void __dt__5CTaskFv(void*, int);
void LoadingScreen_LoadB2dResources();
void GolfTaskTree_FreeSubtreeNodes(void*, void*);
void InputManager_Dtor();
void String_ReserveCapacity(void*, unsigned long);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);

extern const char golfTaskActorStrings[];
extern int g_sharedDoc;
extern char s_roundPlayerList[];
signed char s_inputManagerInitialized;
extern unsigned char s_gameInputManager[];
extern unsigned char s_inputManagerDtorRegistration_GolfMenu[];
extern unsigned char __vt__9CGolfTask[];
extern int g_defaultActorSkipMask;
extern const float kFloat_neg10_GolfTask;

struct AllocatedObject {
    char storage[0x3F0];
    // Construct the task in this allocation; exception cleanup belongs to the new-expression.
    AllocatedObject() {
        ((CGolfTask*)this)->__ct();
    }
};

enum GolfTaskConstants {
    kDefaultRoundActorSkipMask = 0x7E22021,
    kRoundControllerCount = 4,
    kPreloadOwnerCount = 6,
    kDeleteResourceVirtualSlot = 2,
    kGameModeWithStoryActors = 6,
    kPartyDarts = 1,
    kPartyDrive = 2,
    kPartyBalloon = 3,
    kPartyMode6 = 6
};

// Allocate and construct the task that owns round actors, loading UI and resources.
// Returns an owned task; release it through its deleting virtual destructor.
void* CGolfTask_Create() {
    return new AllocatedObject;
}

// Initialize task interfaces, actor tree, loading state and the six resource slots.
CGolfTask::CGolfTask() {
    ((CTask*)this)->Init();
    float initValue = kFloat_neg10_GolfTask;
    unsigned char* b = (unsigned char*)this;
    (*(unsigned char**)&AsCGolfTask(b)->vtable) = __vt__9CGolfTask;
    (*(unsigned char**)&AsCGolfTask(b)->secondaryVtable) = __vt__9CGolfTask + 0x10;
    (*(unsigned char**)&AsCGolfTask(b)->tertiaryVtable) = __vt__9CGolfTask + 0x1C;
    AsCGolfTask(b)->treeNodeCount = 0;
    AsCGolfTask(b)->treeRoot = 0;
    AsCGolfTask(b)->treeSentinel = b + 0x3AC;
    AsCGolfTask(b)->pauseScoreMenu = 0;
    AsCGolfTask(b)->messageActive = 0;
    AsCGolfTask(b)->loadingDraw = 0;
    AsCGolfTask(b)->loadingDrawFinished = 0;
    AsCGolfTask(b)->variantPreloadFile = 0;
    AsCGolfTask(b)->inGameTexturePack1 = 0;
    AsCGolfTask(b)->inGameTexturePack2 = 0;
    AsCGolfTask(b)->tutorialTexturePack = 0;
    g_defaultActorSkipMask = kDefaultRoundActorSkipMask;
    AsCGolfTask(b)->unk_C = 0;
    AsCGolfTask(b)->delayTimer = initValue;
    memset(resources, 0, sizeof(resources));
}

// Two link words with different interpretations: round list {prev, next}, tree {left, right}.
// The list header is {count, sentinel.prev, sentinel.next}; its payload is not freed here.
// The task tree owns its nodes. The nested destructor below is unrolled postorder cleanup,
// with GolfTaskTree_FreeSubtreeNodes handling deeper branches; it is not a linear list walk.
struct Node {
    void* data;
    Node* next;
};
struct Pair {
    void* first;
    void* second;
};

struct TextAllocator {
    char value;
    // Preserve the empty allocator token; no meaningful state is initialized here.
    TextAllocator() {}
    // Copy the ABI token byte, including the original volatile access.
    TextAllocator(const TextAllocator& other) : value(*(volatile const char*)&other) {}
};
// Temporary string ABI: three words; a's high bit owns heap storage addressed by c.
// For inline text, byte0 & 0x7F is length and characters begin at +1; for heap text,
// b is length and c is the allocation. The consumer in task_actor_loading.cpp reads this exact ABI.
struct TextArg {
    int a, b, c;
    // Replace the initially empty range with the borrowed character range [begin, end).
    void assign(const char* begin, const char* end, TextAllocator allocator) {
        String_ReplaceRange(this, 0, 0, begin, end, (char*)&allocator);
    }
    // Reserve for the byte range, then copy it with the allocator token.
    void initialize(const char* text, const char* end, const TextAllocator& allocator) {
        unsigned long len = end - text;
        String_ReserveCapacity(this, len);
        assign(text, text + len, allocator);
    }
    // Build an owned temporary from a NUL-terminated string.
    TextArg(const char* text, const TextAllocator& allocator) : a(0), b(0), c(0) {
        initialize(text, text + strlen(text), allocator);
    }
    // Free only heap-backed temporary text; inline storage needs no release.
    ~TextArg() {
        if ((unsigned int)a >> 31)
            appFreeUni__FPv((void*)c);
    }
};

// Create an actor using class-name/instance-name byte offsets in golfTaskActorStrings.
// The task retains the actor. The empty option-string temporary is borrowed only for
// creation and is destroyed immediately afterward, including during exception unwinding.
inline void* CreateGolfActor(void* task, int classOffset, int instanceOffset, int clearedSkipBits) {
    return TaskActorTable_CreateNamedActor(task, golfTaskActorStrings + classOffset,
                                           golfTaskActorStrings + instanceOffset, clearedSkipBits,
                                           &TextArg(golfTaskActorStrings, TextAllocator()));
}

// Populate the task's named actor table, selecting extra actors by story and party mode.
// Each call passes an empty option string. Masks CLEAR actor skip bits/combinations
// through IActor_ClearSkipFlags in TaskActorTable_CreateNamedActor;
// individual bit meanings are unresolved here, so preserve each mask when adding actors.
void CGolfTask::CreateActors() {
    // Phase 1: install the main rule actor, shared clock/gallery/camera, then story-only actors.
    ((CTask*)this)
        ->SetMainActor(TaskActorTable_CreateNamedActor(this, golfTaskActorStrings + 1, golfTaskActorStrings + 0xb,
                                                       0x04020021, &TextArg(golfTaskActorStrings, TextAllocator())));
    CreateGolfActor(this, 0x14, 0x1b, 0x20); // CClock / Clock
    CreateGolfActor(this, 0x21, 0x2a, 0);    // CGallery / Gallery
    CreateGolfActor(this, 0x32, 0x3a, 0x20); // CCamera / Camera
    if (AsCSharedDoc(g_sharedDoc)->gameMode == kGameModeWithStoryActors) {
        CreateGolfActor(this, 0x41, 0x4e, 0x400000);  // CConteSelect / ConteSelect
        CreateGolfActor(this, 0x5a, 0x66, 0x1000000); // CStoryConte / StoryConte
        CreateGolfActor(this, 0x71, 0x80, 0x4000000); // CStoryGameOver / StoryGameOver
    }
    // Phase 2: world/player presentation. Darts omits the grid actor.
    CreateGolfActor(this, 0x8e, 0x96, 0x05022001); // CGolfBg / GolfBg
    CreateGolfActor(this, 0x9d, 0xa9, 0x05022000); // CGolfPlayer / Player
    CreateGolfActor(this, 0xb0, 0xbc, 0x1000000);  // CGolfCaddie / Caddie
    CreateGolfActor(this, 0xc3, 0xc8, 0x01000001); // CNPC / NPC
    if (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode != kPartyDarts) {
        CreateGolfActor(this, 0xcc, 0xd2, 0x20); // CGrid / Grid
    }
    CreateGolfActor(this, 0xd7, 0xdd, 0x20);         // CSpot / Spot
    CreateGolfActor(this, 0xe2, 0xe8, 0x2000);       // CBall / Ball
    CreateGolfActor(this, 0xed, 0xf8, 0x20);         // CGuideLine / GuideLine
    CreateGolfActor(this, 0x102, 0x10e, 0);          // CTrajectory / Trajectory
    CreateGolfActor(this, 0x119, 0x129, 0x04020001); // CParticleEffect / Effect
    CreateGolfActor(this, 0x130, 0x134, 0x04020001); // CFx / Fx
    CreateGolfActor(this, 0x137, 0x145, 1);          // CAmbientSound / Ambient
    CreateGolfActor(this, 0x14d, 0x157, 0x20);       // CTutorial / Tutorial
    // Phase 3: party mode 1/2/3 selects Darts/Drive/BalloonSystem; mode 6 adds none here.
    switch (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) {
    case kPartyDarts:
        CreateGolfActor(this, 0x160, 0x16d, 0x00020020);
        break;
    case kPartyDrive:
        CreateGolfActor(this, 0x179, 0x186, 0x20);
        break;
    case kPartyBalloon:
        CreateGolfActor(this, 0x192, 0x1a1, 0x20);
        break;
    case kPartyMode6:
        break;
    }
    // Phase 4: common HUD, score and voice actors are present in every branch above.
    CreateGolfActor(this, 0x1af, 0x1b7, 0x20);       // CBubble / Bubble
    CreateGolfActor(this, 0x1be, 0x1c6, 0x04020020); // CLetter / Letter
    CreateGolfActor(this, 0x1cd, 0x1d5, 0x2020);     // CScreen / Screen
    CreateGolfActor(this, 0x1dc, 0x1e8, 0x2000);     // CItemWindow / Item
    CreateGolfActor(this, 0x1ed, 0x1f3, 0);          // CSpin / Spin
    CreateGolfActor(this, 0x1f8, 0x1fe, 1);          // CLogo / Logo
    CreateGolfActor(this, 0x203, 0x20a, 0x04222020); // CScore / Score
    CreateGolfActor(this, 0x210, 0x21e, 0x04020000); // CVoiceManager / Voice
}

} // extern "C"

// Detach and free the round player list, clear the cursor overrides, release
// the preload resources, then free the actor tree bottom-up.
// Node allocations and resource owners are released, not player payloads.
#pragma use_lmw_stmw on
CGolfTask::~CGolfTask() {
    vtable = __vt__9CGolfTask;
    secondaryVtable = __vt__9CGolfTask + 0x10;
    tertiaryVtable = __vt__9CGolfTask + 0x1C;
    // Phase 1: detach and free the complete round-list node range, preserving the sentinel.
    Node* node = *(Node**)((char*)&s_roundPlayerList + 8);
    Node* sentinel = (Node*)((char*)&s_roundPlayerList + 4);
    if (node != sentinel) {
        Node* last = (Node*)sentinel->data;
        Node* previous = (Node*)node->data;
        previous->next = last->next;
        last->next->data = node->data;
    }
    while (node != sentinel) {
        Node* next = node->next;
        appFreeUni__FPv(node);
        --*(int*)&s_roundPlayerList;
        node = next;
    }
    // Phase 2: obtain the input singleton and reset all four controller cursor overrides.
    if (!((int&)GetInputManagerSlot())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_GolfMenu);
            s_inputManagerInitialized = 1;
        }
        ((int&)GetInputManagerSlot()) = (int)s_gameInputManager;
    }
    void* controller = (void*)((int&)GetInputManagerSlot());
    for (int i = 0; i < kRoundControllerCount; ++i) {
        InputManager_SetCursorDrawOptions(controller, 0, 0, 0, i);
        InputManager_SetCursorColorOverride(controller, 0, 0xD6, 0x48, 0x5C, i);
    }
    // Phase 3: deleting virtual calls consume each preload owner, then clear its slot.
    for (unsigned int i = 0; i < kPreloadOwnerCount; ++i) {
        if (resources[i]) {
            if (resources[i])
                ((void (*)(void*, int))(*(void***)resources[i])[kDeleteResourceVirtualSlot])(resources[i], 1);
            resources[i] = 0;
        }
    }
    {
        void** slot = (void**)&inGameTexturePack1;
        if (*slot) {
            if (*slot)
                ((void (*)(void*, int))(*(void***)*slot)[kDeleteResourceVirtualSlot])(*slot, 1);
        }
        *slot = 0;
    }
    {
        void** slot = (void**)&inGameTexturePack2;
        if (*slot) {
            if (*slot)
                ((void (*)(void*, int))(*(void***)*slot)[kDeleteResourceVirtualSlot])(*slot, 1);
        }
        *slot = 0;
    }
    {
        void** slot = (void**)&tutorialTexturePack;
        if (*slot) {
            if (*slot)
                ((void (*)(void*, int))(*(void***)*slot)[kDeleteResourceVirtualSlot])(*slot, 1);
        }
        *slot = 0;
    }
    {
        void** slot = (void**)&variantPreloadFile;
        if (*slot) {
            if (*slot)
                ((void (*)(void*, int))(*(void***)*slot)[kDeleteResourceVirtualSlot])(*slot, 1);
        }
        *slot = 0;
    }
    // Phase 4: restore loading-screen resources, then free the binary tree bottom-up.
    LoadingScreen_LoadB2dResources();
    // The tree member's destructor is expanded here; the original tests the
    // member's address once for each inlined level before walking the nodes.
    if (&treeNodeCount != 0 && &treeNodeCount != 0 && &treeNodeCount != 0 && &treeNodeCount != 0 &&
        &treeNodeCount != 0 && &treeNodeCount != 0) {
        Node* root = (Node*)treeRoot;
        if (root) {
            Node* left = (Node*)root->data;
            if (left) {
                Node* leftLeft = (Node*)left->data;
                if (leftLeft) {
                    Node* childSubtree = (Node*)leftLeft->data;
                    if (childSubtree) {
                        void* leftDescendant = *(void**)childSubtree;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = childSubtree->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(childSubtree);
                    }
                    childSubtree = (Node*)leftLeft->next;
                    if (childSubtree) {
                        void* leftDescendant = *(void**)childSubtree;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = childSubtree->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(childSubtree);
                    }
                    appFreeUni__FPv(leftLeft);
                }
                Node* leftRight = (Node*)left->next;
                if (leftRight) {
                    Node* leftRightLeft = (Node*)leftRight->data;
                    if (leftRightLeft) {
                        void* leftDescendant = *(void**)leftRightLeft;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = leftRightLeft->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(leftRightLeft);
                    }
                    Node* leftRightRight = (Node*)leftRight->next;
                    if (leftRightRight) {
                        void* leftDescendant = *(void**)leftRightRight;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = leftRightRight->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(leftRightRight);
                    }
                    appFreeUni__FPv(leftRight);
                }
                appFreeUni__FPv(left);
            }
            Node* right = (Node*)root->next;
            if (right) {
                Node* rightLeft = (Node*)right->data;
                if (rightLeft) {
                    Node* rightLeftChild = (Node*)rightLeft->data;
                    if (rightLeftChild) {
                        void* leftDescendant = *(void**)rightLeftChild;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = rightLeftChild->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(rightLeftChild);
                    }
                    Node* rightLeftRight = (Node*)rightLeft->next;
                    if (rightLeftRight) {
                        void* leftDescendant = *(void**)rightLeftRight;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = rightLeftRight->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(rightLeftRight);
                    }
                    appFreeUni__FPv(rightLeft);
                }
                Node* rightRight = (Node*)right->next;
                if (rightRight) {
                    Node* rightRightLeft = (Node*)rightRight->data;
                    if (rightRightLeft) {
                        void* leftDescendant = *(void**)rightRightLeft;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = rightRightLeft->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(rightRightLeft);
                    }
                    Node* rightRightRight = (Node*)rightRight->next;
                    if (rightRightRight) {
                        void* leftDescendant = *(void**)rightRightRight;
                        if (leftDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, leftDescendant);
                        void* rightDescendant = rightRightRight->next;
                        if (rightDescendant)
                            GolfTaskTree_FreeSubtreeNodes((char*)&treeNodeCount, rightDescendant);
                        appFreeUni__FPv(rightRightRight);
                    }
                    appFreeUni__FPv(rightRight);
                }
                appFreeUni__FPv(right);
            }
            appFreeUni__FPv(root);
        }
    }
    // Phase 5: the task base tears down its own actor table.
    __dt__5CTaskFv(this, 0);
}
#pragma use_lmw_stmw reset
