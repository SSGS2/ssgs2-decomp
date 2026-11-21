#include "classes/TreeStringNode8022C490.h"
// View the borrowed object using its verified field offsets.
static inline TreeStringNode8022C490* AsTreeStringNode8022C490(void* object) { return (TreeStringNode8022C490*)object; }
#include "classes/CommandState8022D190.h"
// View the borrowed object using its verified field offsets.
static inline CommandState8022D190* AsCommandState8022D190(void* object) { return (CommandState8022D190*)object; }
#pragma use_lmw_stmw on
#include "sdk_protos.h"
#include "game/golfball.h"
// Translation unit 0x8022C490-0x8022DBE4 (15 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern "C" char comThinkCommandStrings[];
int SharedFile_Create(int, int);
int CSharedFilePtr_GetLoadedBuffer(int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void ComThink_CheckAndCacheBoost(void*);
void ComThink_CheckAndCacheCarry(void*);
void ComThink_CheckAndCacheAreaStop(void*);
void* GolfBall__Fv(void);
int ComThink_CommandDebugPrintStub(const char*, ...);
int String_ReplaceRange(void*, int, unsigned int, const char*, const char*, const char*);
// ORIGINAL: String_ReserveCapacity(this, 0): r4 is an explicit `li r4,0` before the call (the code is dead on the Wii: the
// heap branch is never taken).
void String_ReserveCapacity(void*, unsigned int);
void* ComConditionMap_InsertNode(void*, int, int, int, void*);
void* ComActionMap_InsertNode(void*, void*, int, int, void*);
void* ComThinkActionTree_FindInsertionPosition(void*, int*, void**, char*, char*);
void* ComThinkConditionTree_FindInsertionPosition(void*, int*, void**, char*, char*);
int CComThinkCtrl_FindNextEntryByName(void*, void*, int*);
void ComConditionMap_FreeSubtree(void*, void*);
void Tree_FreeSubtree(void*, void*);
void* memcpy(void*, const void*, unsigned int);

struct CommandStringTree {
    int count;
    void* root;
    int sentinelParent;
    void* first;
};
struct ThinkTreeHeader { unsigned int count; void* root; void* sentinelParent; void* first; };
struct ThinkControllerTrees { void* vtable; ThinkTreeHeader conditions; ThinkTreeHeader actions; };
typedef int (*ThinkConditionCallback)(void* request);
typedef int (*ThinkActionCallback)(void* request, void* shotFlags);
struct ThinkRequest {
    int entryId;
    int condition;
    int conditionArguments[3];
    int action;
    int actionArguments[3];
};
typedef char ThinkRequest_size[(sizeof(ThinkRequest) == 0x24) ? 1 : -1];
typedef char ThinkControllerTrees_actionRootOffset[((int)&((ThinkControllerTrees*)0)->actions.root == 0x18) ? 1 : -1];
enum ThinkCommandConstants {
    kCommandCount = 48, kCommandArrayBytes = 48 * 4, kCheckpointBoundary = 11,
    kCommandRejected = 2, kCommandAccepted = 3,
    kBoostCommand = 14, kCarryCommand = 16, kAreaStopCommand = 17,
    kConditionNameFormat = 0x286, kConditionArgumentsFormat = 0x28E,
    kConditionRejectedText = 0x2A2, kConditionAcceptedText = 0x2AB,
    kActionNameFormat = 0x2B4, kActionArgumentsFormat = 0x2C3,
    kActionSucceededText = 0x2DE, kActionFailedText = 0x2E6
};

// Command slots are indexed by the same 48 command IDs as ThinkRequest.
struct ThinkCheckpointSlots128 {
    char prefix[0x2C];
    int conditionResults[kCommandCount];
    int actionResults[kCommandCount];
    int accumulator1AC, accumulator1B0, accumulator1B4; // consumer meanings unresolved
};
typedef char ThinkCheckpointSlots128_actions[((int)&((ThinkCheckpointSlots128*)0)->actionResults == 0xEC) ? 1 : -1];

// The controller owns two integer-key trees: condition callbacks return a command
// status (2 rejects), action callbacks return a boolean and may write shotFlags.
// Requests borrow their argument arrays. Keys must be registered before evaluation;
// a failed lookup here reaches the header sentinel rather than a safe default.
// Each header is {count, root, sentinel parent/color, cached first node}; &root is
// the empty-tree sentinel. Nodes have links at +0/+4, parent/color at +8, key at
// +0xC, a 12-byte small/owned string at +0x10, and callback word at +0x1C.

// The first three child levels are expanded destructor calls: recurse only below
// each leaf, release its owned text, then release the node. The repeated
// node+0xC/node+0x10 tests come from nested pair/string destructor guards;
// they are member-address checks, not additional tree links or validity tests.
// stringFlags bit31 alone selects owned heap text; inline text is part of the node.
// Free the string-key tree in postorder and restore its empty sentinel.
// Consume a string-tree leaf after deeper descendants and its owned label.
static inline void FreeThinkStringLeaf128(CommandStringTree* self, char* node) {
    if (node) {
        if (AsTreeStringNode8022C490(node)->left) Tree_FreeSubtree(self, AsTreeStringNode8022C490(node)->left);
        if (AsTreeStringNode8022C490(node)->right) Tree_FreeSubtree(self, AsTreeStringNode8022C490(node)->right);
        if (node + 0xC && node + 0x10 && node + 0x10 && (AsTreeStringNode8022C490(node)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(node)->heapText);
        appFreeUni__FPv(node);
    }
}

// Free the owned command-label tree and restore its empty sentinel.
void CComThinkCtrl_ClearStringTree(CommandStringTree* self) {
    // Phase 1: release the unrolled child levels in postorder, then the root.
    char* root = (char*)self->root;
    if (root) {
        char* rootLeft = (char*)AsTreeStringNode8022C490(root)->left;
        if (rootLeft) {
            char* rootLeftLeft = (char*)AsTreeStringNode8022C490(rootLeft)->left;
            if (rootLeftLeft) {
                char* leftLeaf = (char*)AsTreeStringNode8022C490(rootLeftLeft)->left;
                FreeThinkStringLeaf128(self, leftLeaf);
                char* leafCursor = (*(char**)&AsTreeStringNode8022C490(rootLeftLeft)->right);
                FreeThinkStringLeaf128(self, leafCursor);
                if (rootLeftLeft + 0xC && rootLeftLeft + 0x10 && rootLeftLeft + 0x10 && (AsTreeStringNode8022C490(rootLeftLeft)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootLeftLeft)->heapText);
                appFreeUni__FPv(rootLeftLeft);
            }
            char* rootLeftRight = (*(char**)&AsTreeStringNode8022C490(rootLeft)->right);
            if (rootLeftRight) {
                char* leftLeaf = (char*)AsTreeStringNode8022C490(rootLeftRight)->left;
                FreeThinkStringLeaf128(self, leftLeaf);
                char* leafCursor = (*(char**)&AsTreeStringNode8022C490(rootLeftRight)->right);
                FreeThinkStringLeaf128(self, leafCursor);
                if (rootLeftRight + 0xC && rootLeftRight + 0x10 && rootLeftRight + 0x10 && (AsTreeStringNode8022C490(rootLeftRight)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootLeftRight)->heapText);
                appFreeUni__FPv(rootLeftRight);
            }
            if (rootLeft + 0xC && rootLeft + 0x10 && rootLeft + 0x10 && (AsTreeStringNode8022C490(rootLeft)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootLeft)->heapText);
            appFreeUni__FPv(rootLeft);
        }

        char* rootRight = (*(char**)&AsTreeStringNode8022C490(root)->right);
        if (rootRight) {
            char* rootRightLeft = (char*)AsTreeStringNode8022C490(rootRight)->left;
            if (rootRightLeft) {
                char* leafCursor = (char*)AsTreeStringNode8022C490(rootRightLeft)->left;
                FreeThinkStringLeaf128(self, leafCursor);
                char* rightLeaf = (*(char**)&AsTreeStringNode8022C490(rootRightLeft)->right);
                FreeThinkStringLeaf128(self, rightLeaf);
                if (rootRightLeft + 0xC && rootRightLeft + 0x10 && rootRightLeft + 0x10 && (AsTreeStringNode8022C490(rootRightLeft)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootRightLeft)->heapText);
                appFreeUni__FPv(rootRightLeft);
            }
            char* rootRightRight = (*(char**)&AsTreeStringNode8022C490(rootRight)->right);
            if (rootRightRight) {
                char* leafCursor = (char*)AsTreeStringNode8022C490(rootRightRight)->left;
                FreeThinkStringLeaf128(self, leafCursor);
                char* rightLeaf = (*(char**)&AsTreeStringNode8022C490(rootRightRight)->right);
                FreeThinkStringLeaf128(self, rightLeaf);
                if (rootRightRight + 0xC && rootRightRight + 0x10 && rootRightRight + 0x10 && (AsTreeStringNode8022C490(rootRightRight)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootRightRight)->heapText);
                appFreeUni__FPv(rootRightRight);
            }
            if (rootRight + 0xC && rootRight + 0x10 && rootRight + 0x10 && (AsTreeStringNode8022C490(rootRight)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(rootRight)->heapText);
            appFreeUni__FPv(rootRight);
        }

        if (root + 0xC && root + 0x10 && root + 0x10 && (AsTreeStringNode8022C490(root)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(root)->heapText);
        appFreeUni__FPv(root);
        // Phase 2: after all payload/node frees, restore the empty header sentinel.
        self->count = 0;
        self->root = 0;
        self->first = &self->root;
    }
}

// Map header (shared by the condition and action maps): +0 and +4 are reset to 0 (+4 = root pointer; the header
// address &root doubles as the end()/sentinel node in lookups), +0xC = leftmost-node cache (points at &root when empty).
// Node (TreeStringNode8022C490): left +0, right +4, parentAndColor +8, int key +0xC, name string +0x10..0x1B
// (flag bit 31 set = heap text pointer at +0x18, else inline chars at node+0x11), callback word `value` +0x1C.
// Free all condition records and owned strings, then reset the map sentinel.
// A nonempty clear returns 0; the empty path has no explicit return. Do not use
// the return value as a status. An already-empty header is left untouched.
// Consume a condition leaf after deeper descendants and its owned display string.
static inline void FreeThinkConditionLeaf128(void* self, char* node) {
    if (node) {
        if (AsTreeStringNode8022C490(node)->left) ComConditionMap_FreeSubtree(self, AsTreeStringNode8022C490(node)->left);
        if (AsTreeStringNode8022C490(node)->right) ComConditionMap_FreeSubtree(self, AsTreeStringNode8022C490(node)->right);
        if (node + 0xC && node + 0x10 && node + 0x10 && (AsTreeStringNode8022C490(node)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(node)->heapText);
        appFreeUni__FPv(node);
    }
}

// Clear all condition records, then restore the root-slot sentinel.
int ComConditionMap_Clear(void* self) {
    // branch/child/grandchild are reused at each depth for left, then right.
    // Both descendants are gone before freeing the current string and node.
    // Phase 1: release the unrolled child levels in postorder, then the root.
    char* root;
    char* branch;
    char* child;
    char* grandchild;
    root = *(char**)((char*)self + 4);
    if (root) {
        branch = (char*)AsTreeStringNode8022C490(root)->left;
        if (branch) {
            child = (char*)AsTreeStringNode8022C490(branch)->left;
            if (child) {
                grandchild = (char*)AsTreeStringNode8022C490(child)->left;
                FreeThinkConditionLeaf128(self, grandchild);
                grandchild = (*(char**)&AsTreeStringNode8022C490(child)->right);
                FreeThinkConditionLeaf128(self, grandchild);
                if (child + 0xC && child + 0x10 && child + 0x10 && (AsTreeStringNode8022C490(child)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(child)->heapText);
                appFreeUni__FPv(child);
            }
            child = (*(char**)&AsTreeStringNode8022C490(branch)->right);
            if (child) {
                grandchild = (char*)AsTreeStringNode8022C490(child)->left;
                FreeThinkConditionLeaf128(self, grandchild);
                grandchild = (*(char**)&AsTreeStringNode8022C490(child)->right);
                FreeThinkConditionLeaf128(self, grandchild);
                if (child + 0xC && child + 0x10 && child + 0x10 && (AsTreeStringNode8022C490(child)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(child)->heapText);
                appFreeUni__FPv(child);
            }
            if (branch + 0xC && branch + 0x10 && branch + 0x10 && (AsTreeStringNode8022C490(branch)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(branch)->heapText);
            appFreeUni__FPv(branch);
        }

        branch = (*(char**)&AsTreeStringNode8022C490(root)->right);
        if (branch) {
            child = (char*)AsTreeStringNode8022C490(branch)->left;
            if (child) {
                grandchild = (char*)AsTreeStringNode8022C490(child)->left;
                FreeThinkConditionLeaf128(self, grandchild);
                grandchild = (*(char**)&AsTreeStringNode8022C490(child)->right);
                FreeThinkConditionLeaf128(self, grandchild);
                if (child + 0xC && child + 0x10 && child + 0x10 && (AsTreeStringNode8022C490(child)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(child)->heapText);
                appFreeUni__FPv(child);
            }
            child = (*(char**)&AsTreeStringNode8022C490(branch)->right);
            if (child) {
                grandchild = (char*)AsTreeStringNode8022C490(child)->left;
                FreeThinkConditionLeaf128(self, grandchild);
                grandchild = (*(char**)&AsTreeStringNode8022C490(child)->right);
                FreeThinkConditionLeaf128(self, grandchild);
                if (child + 0xC && child + 0x10 && child + 0x10 && (AsTreeStringNode8022C490(child)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(child)->heapText);
                appFreeUni__FPv(child);
            }
            if (branch + 0xC && branch + 0x10 && branch + 0x10 && (AsTreeStringNode8022C490(branch)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(branch)->heapText);
            appFreeUni__FPv(branch);
        }

        if (root + 0xC && root + 0x10 && root + 0x10 && (AsTreeStringNode8022C490(root)->stringFlags >> 31)) appFreeUni__FPv(AsTreeStringNode8022C490(root)->heapText);
        appFreeUni__FPv(root);
        // Phase 2: restore the empty sentinel only after all nodes are released.
        char* headerBytes = (char*)self;
        *(void**)headerBytes = 0;
        *(void**)(headerBytes + 4) = 0;
        *(void**)(headerBytes + 0xC) = headerBytes + 4;
        return 0;
    }
}

#pragma push
#pragma opt_common_subs off
// Evaluate one condition, cache its result, then run and record the selected action.
// Lookups below are lower_bound searches by integer key on the condition and action maps; a miss resolves to the
// header sentinel (&tree.root); the code then still reads name/value through that pointer (end-node contents not verified).
// request: condition/action keys, plus 3 argument words each, are read from the ThinkRequest.
// Callbacks: condition(request) -> result (kCommandRejected stops everything, return 0);
//            action(request, &shotFlags) -> nonzero = success, which sets commandFlags[actionKey] = 1.
// Returns 1 when the condition was accepted (even if the action failed), 0 when it was rejected.
int CComThinkCtrl_ProcessRequest(void* self, void* request) {
    // Phase 1: resolve the condition, log its arguments, and stop on rejection.
    int conditionKey = ((ThinkRequest*)request)->condition;
    char* conditionNode = (char*)&((ThinkControllerTrees*)self)->conditions.root;
    char* conditionCursor = *(char**)conditionNode;
    while (conditionCursor) {
        if (AsTreeStringNode8022C490(conditionCursor)->key >= conditionKey) { conditionNode = conditionCursor; conditionCursor = *(char**)conditionCursor; }
        else conditionCursor = (*(char**)&AsTreeStringNode8022C490(conditionCursor)->right);
    }
    if (conditionNode == (char*)&((ThinkControllerTrees*)self)->conditions.root || conditionKey < AsTreeStringNode8022C490(conditionNode)->key) conditionNode = (char*)&((ThinkControllerTrees*)self)->conditions.root;
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + kConditionNameFormat, (AsTreeStringNode8022C490(conditionNode)->stringFlags >> 31) == 0 ? conditionNode + 0x11 : (*(char**)&AsTreeStringNode8022C490(conditionNode)->heapText));
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + kConditionArgumentsFormat, ((ThinkRequest*)request)->conditionArguments[0], ((ThinkRequest*)request)->conditionArguments[1], ((ThinkRequest*)request)->conditionArguments[2]);
    // The node value is a borrowed function address. Condition callbacks take the
    // complete request; action callbacks also receive the writable shot-flag word.
    int result = ((ThinkConditionCallback)AsTreeStringNode8022C490(conditionNode)->value)(request);
    if (result == kCommandRejected) { ComThink_CommandDebugPrintStub(comThinkCommandStrings + kConditionRejectedText); return 0; }
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + kConditionAcceptedText);
    int conditionResultByteOffset = conditionKey * 4;
    int actionKey = ((ThinkRequest*)request)->action;
    AsCommandState8022D190(self)->commandResults[conditionKey] = result; // 0x8022D23C: results +0xEC
    // Phase 2: resolve the action. Failure leaves its flag clear but still returns
    // success: this function reports condition acceptance, not action success.
    char* actionNode = (char*)&((ThinkControllerTrees*)self)->actions.root;
    char* actionCursor = *(char**)actionNode;
    while (actionCursor) {
        if (AsTreeStringNode8022C490(actionCursor)->key >= actionKey) { actionNode = actionCursor; actionCursor = *(char**)actionCursor; }
        else actionCursor = (*(char**)&AsTreeStringNode8022C490(actionCursor)->right);
    }
    if (actionNode == (char*)&((ThinkControllerTrees*)self)->actions.root || actionKey < AsTreeStringNode8022C490(actionNode)->key) actionNode = (char*)&((ThinkControllerTrees*)self)->actions.root;
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + kActionNameFormat, (AsTreeStringNode8022C490(actionNode)->stringFlags >> 31) == 0 ? actionNode + 0x11 : (*(char**)&AsTreeStringNode8022C490(actionNode)->heapText));
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + kActionArgumentsFormat, ((ThinkRequest*)request)->actionArguments[0], ((ThinkRequest*)request)->actionArguments[1], ((ThinkRequest*)request)->actionArguments[2]);
    if (((ThinkActionCallback)AsTreeStringNode8022C490(actionNode)->value)(request, &AsCommandState8022D190(self)->shotFlags)) {
        AsCommandState8022D190(self)->commandFlags[actionKey] = 1; // 0x8022D23C: flags +0x2C
        ComThink_CommandDebugPrintStub(comThinkCommandStrings + kActionSucceededText);
    } else ComThink_CommandDebugPrintStub(comThinkCommandStrings + kActionFailedText);
    return 1;
}
#pragma pop

// Evaluate named requests until the checkpoint boundary; reset shot outputs after a rejected condition.
// input is a borrowed NUL-terminated name. Optional output must hold 96 words:
// 48 action flags followed by 48 condition results; shot parameters stay in self.
// Empty input succeeds after reset and leaves output untouched. Returns 0 only
// when a rejected segment has not reached another boundary before input ends.
int CComThinkCtrl_EvaluateCheckpoint(void* self, const char* input, void* output) {
    char* commandState = (char*)self;
    int enabledValue = 1;
    AsCommandState8022D190(commandState)->shotFlags = 0;
    AsCommandState8022D190(commandState)->shotCurve = 0.0f;
    AsCommandState8022D190(commandState)->shotSpin = 0.0f;
    AsCommandState8022D190(commandState)->powerShotLevel = 0;
    AsCommandState8022D190(commandState)->difficultyEnabled = (char)enabledValue;
    AsCommandState8022D190(commandState)->clubIndex = -1;
    // Phase 1: clear action slots, then condition slots; reset shot outputs and scratch words.
    for (int i = (int)&((ThinkCheckpointSlots128*)0)->actionResults; i <= (int)&((ThinkCheckpointSlots128*)0)->actionResults[kCommandCount - 1]; i += sizeof(int)) *(int*)(commandState + i) = 0;
    for (int i = (int)&((ThinkCheckpointSlots128*)0)->conditionResults; i <= (int)&((ThinkCheckpointSlots128*)0)->conditionResults[kCommandCount - 1]; i += sizeof(int)) *(int*)(commandState + i) = 0;
    // The three checkpoint accumulator words are reset separately from command results.
    AsCommandState8022D190(commandState)->unk_1AC = 0;
    AsCommandState8022D190(commandState)->unk_1B0 = 0;
    AsCommandState8022D190(commandState)->unk_1B4 = 0;
    if (*input == 0) return 1;

    int entryCursor[2];
    entryCursor[0] = -1;
    ComThink_CommandDebugPrintStub(comThinkCommandStrings + 0x256);
    float resetScalar = 0.0f;
    // Phase 2: evaluate a segment up to boundary command 11. A rejected entry
    // discards shot settings and skips to the next boundary, then retries there.
    // End-of-input while skipping fails; otherwise publish the accepted arrays.
    int skippingRejectedSegment = 0;
    while (CComThinkCtrl_FindNextEntryByName(self, (void*)input, entryCursor)) {
        void* entryRequest = (void*)entryCursor[1];
        if (skippingRejectedSegment != 0) {
            if (((ThinkRequest*)entryRequest)->condition != kCheckpointBoundary) continue;
            skippingRejectedSegment = 0;
        } else {
            if (((ThinkRequest*)entryRequest)->condition == kCheckpointBoundary) break;
        }
        int result = CComThinkCtrl_ProcessRequest(self, entryRequest);
        if (!result) {
            AsCommandState8022D190(commandState)->shotFlags = 0;
            skippingRejectedSegment = enabledValue;
            AsCommandState8022D190(commandState)->shotCurve = resetScalar;
            AsCommandState8022D190(commandState)->shotSpin = resetScalar;
            AsCommandState8022D190(commandState)->powerShotLevel = 0;
            AsCommandState8022D190(commandState)->difficultyEnabled = (char)enabledValue;
            AsCommandState8022D190(commandState)->clubIndex = -1;
        }
    }
    if (skippingRejectedSegment) return 0;
    // Phase 3: publish only the two command arrays after a successful segment.
    if (output) {
        int* s = (int*)(commandState + 0x28);
        int* d = (int*)((char*)output - 4);
        int n = 24;
        do {
            int a = s[1];
            int b = s[2];
            s += 2;
            d[1] = a;
            d[2] = b;
            d += 2;
        } while (--n);
        s = (int*)(commandState + 0xE8);
        d = (int*)((char*)output + kCommandArrayBytes - 4);
        n = 24;
        do {
            int a = s[1];
            int b = s[2];
            s += 2;
            d[1] = a;
            d[2] = b;
            d += 2;
        } while (--n);
    }
    return 1;
}

// Glossary for the legacy D7 names: D7String = engine string (12 bytes), D7Value = string + callback word
// (CommandCallbackRecord), D7Pair = int command key + D7Value (CommandKeyValue), D7Alloc = allocator tag byte
// (CommandAllocatorTag), D7Walk = tree descent for insertion. Registration copies a pair into a new tree node;
// the node then owns the string heap data (if any) and the callback word is a borrowed function address.
// Registration records: D7String owns its text, D7Value adds a callback word,
// and D7Pair adds the integer command key. D7 is a legacy helper prefix.
// String storage is 12 bytes: inline length/tag at +0 and chars at +1;
// heap mode uses bit31, length at +4, and owned character pointer at +8.
// Allocator bytes are ABI tags, not character data; retain their original lifetime.
struct D7Alloc { char byte; };
struct D7Words { unsigned int a, b, c; };
struct D7String {
    union {
    char raw[12];
    struct {
    unsigned int bits;
    unsigned int length;
    char* data;
    };
    };
    // Initialize an empty inline string without allocating.
    D7String() : bits(0), length(0), data(0) {}
    // Copy borrowed text using the engine string ABI and allocator tag.
    D7String(const char* text, const D7Alloc& alloc) {
        ((volatile D7Words*)this)->a = 0; ((volatile D7Words*)this)->b = 0; ((volatile D7Words*)this)->c = 0;
        unsigned int size = *(volatile unsigned char*)raw & 0x7f;
        D7Alloc a;
        a.byte = ((const volatile D7Alloc*)&alloc)->byte;
        unsigned int n = strlen(text);
        String_ReplaceRange(this, 0, size, text, text + n, (char*)&a);
    }
    // Replace the current inline contents with borrowed text.
    void set(const char* text, const D7Alloc& alloc) {
        unsigned int size = (unsigned char)raw[0] & 0x7f;
        D7Alloc a(alloc);
        unsigned int n = strlen(text);
        String_ReplaceRange(this, 0, size, text, text + n, (char*)&a);
    }
    // Copy inline words directly; otherwise replace contents with a deep copy.
    void assign(const D7String& other) {
        unsigned int isLong = bits >> 31;
        if (!isLong && !(other.bits >> 31)) {
            *(D7Words*)this = *(const D7Words*)&other;
        } else {
            unsigned int oldlen;
            if (isLong) oldlen = length;
            else oldlen = *(unsigned char*)this & 0x7f;
            const char* src;
            unsigned int srclen;
            if (!(other.bits >> 31)) { src = (const char*)&other + 1; srclen = *(unsigned char*)&other & 0x7f; }
            else { src = other.data; srclen = other.length; }
            D7Alloc a;
            D7Alloc b(a);
            String_ReplaceRange(this, 0, oldlen, src, src + srclen, (char*)&b);
        }
    }
    // Copy inline storage or allocate independent heap-backed contents.
    D7String(const D7String& other) {
        if (!(other.bits >> 31)) {
            *(D7Words*)this = *(const D7Words*)&other;
        } else {
            bits = 0; length = 0; data = 0;
            String_ReserveCapacity(this, 0);
            D7Alloc a;
            D7Alloc b(a);
            String_ReplaceRange(this, 0, 0, other.data, other.data + other.length, (char*)&b);
        }
    }
    // Release only heap-backed characters; inline bytes require no cleanup.
    ~D7String() { if (bits >> 31) appFreeUni__FPv(data); }
};
struct D7Value {
    D7String str;
    int callbackWord;
    // Default-construct the string; callback word is assigned before registration.
    D7Value() {}
    // Construct the record's display text; the caller supplies its callback word.
    D7Value(const char* text, const D7Alloc& alloc) : str(text, alloc) {}
};
struct D7Pair {
    int key;
    D7Value value;
    // Copy a key and its string/callback record for tree insertion.
    D7Pair(int k, const D7Value& v) : key(k), value(v) {}
};
// Readable roles for the original string/record constructor types.
typedef D7Alloc CommandAllocatorTag;
typedef D7Value CommandCallbackRecord;
typedef D7Pair CommandKeyValue;
// Walk the integer-key tree to the insertion leaf before the modifying lookup.
inline void D7Walk(void* tree, const int& key) {
    int searchKey = key;
    char* node = *(char**)((char*)tree + 4);
    while (node) {
        if (AsTreeStringNode8022C490(node)->key < searchKey) node = (*(char**)&AsTreeStringNode8022C490(node)->right);
        else node = *(char**)node;
    }
}

#pragma push
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Find or create the condition record for an integer key.
inline CommandCallbackRecord& D7AccessB(void* tree, int& key) {
    D7Walk(tree, key);
    void* outNode;
    char flags[2];
    void* current = ComThinkConditionTree_FindInsertionPosition(tree, &key, &outNode, &flags[0], &flags[1]);
    char* slot;
    if (outNode && (slot = (char*)&AsTreeStringNode8022C490(outNode)->key, *(int*)slot >= key)) {
    } else {
        CommandCallbackRecord empty;
        CommandKeyValue pair(key, empty);
        slot = (char*)ComConditionMap_InsertNode(tree, (int)current, (unsigned char)flags[0], (unsigned char)flags[1], &pair) + 0xC;
    }
    return *(CommandCallbackRecord*)(slot + 4);
}
#pragma opt_strength_reduction off
// Copy the display text and borrow the callback address; no ownership of executable code.
// Store a condition's text and callback word under its command key.
int CComThinkCtrl_RegisterCondition(void* self, int key, int callbackWord, const char* text) {
    CommandAllocatorTag alloc;
    CommandCallbackRecord registration(text, alloc);
    registration.callbackWord = callbackWord;
    CommandCallbackRecord& registeredRecord = D7AccessB(&((ThinkControllerTrees*)self)->conditions, key);
    registeredRecord.str.assign(registration.str);
    registeredRecord.callbackWord = registration.callbackWord;
    return 1;
}
#pragma pop
#pragma push
#pragma opt_common_subs off
// Find or create the action record for an integer key.
inline CommandCallbackRecord& D7Access(void* tree, int& key) {
    D7Walk(tree, key);
    void* found;
    char flags[2];
    void* current = ComThinkActionTree_FindInsertionPosition(tree, &key, &found, &flags[0], &flags[1]);
    char* slot;
    if (found && (slot = (char*)&AsTreeStringNode8022C490(found)->key, *(int*)((char*)&AsTreeStringNode8022C490(found)->key) >= key)) {
    } else {
        CommandCallbackRecord empty;
        CommandKeyValue pair(key, empty);
        slot = (char*)ComActionMap_InsertNode(tree, current, (unsigned char)flags[0], (unsigned char)flags[1], &pair) + 0xC;
    }
    return *(CommandCallbackRecord*)(slot + 4);
}
// Store an action's text and callback word under its command key.
int CComThinkCtrl_RegisterAction(void* self, int key, int callbackWord, const char* text) {
    CommandAllocatorTag alloc;
    CommandCallbackRecord registration;
    registration.str.set(text, alloc);
    registration.callbackWord = callbackWord;
    CommandCallbackRecord& registeredRecord = D7Access(&((ThinkControllerTrees*)self)->actions, key);
    registeredRecord.str.assign(registration.str);
    registeredRecord.callbackWord = registration.callbackWord;
    return 1;
}
#pragma pop

// Resolve cached boost/carry/area-stop checks and reject incomplete commands unless the boundary passed.
int CComThinkCtrl_ValidateCommandResults(void* self) {
    if (AsCommandState8022D190(self)->commandResults[kBoostCommand] == kCommandAccepted) ComThink_CheckAndCacheBoost(self);
    if (AsCommandState8022D190(self)->commandResults[kCarryCommand] == kCommandAccepted) ComThink_CheckAndCacheCarry(self);
    if (AsCommandState8022D190(self)->commandResults[kAreaStopCommand] == kCommandAccepted) ComThink_CheckAndCacheAreaStop(self);
    char* p = (char*)self;
    int i;
    for (i = 0; i < kCommandCount; ++i, p += 4) {
        if (AsCommandState8022D190(p)->commandResults[0] == kCommandRejected) {
            if (AsCommandState8022D190(self)->commandResults[kCheckpointBoundary] == kCommandAccepted) {
                ((CGolfBall*)GolfBall__Fv())->m_powerFlags = 0;
                return 1;
            }
            return 0;
        }
    }
    return 1;
}

// Start loading the requested shared file and retain the resulting handle.
int CComThinkCtrl_RequestFileLoad(void* self, int value) {
    int result = SharedFile_Create(value, 0);
    AsCommandState8022D190(self)->sharedFile = result;
    return 1;
}

// Return an insertion parent plus predecessor, left-child flag, and first-node-update flag.
// Output flag 2 stays set only if every descent went left; InsertNode uses it to
// replace the cached leftmost node. It is not a right-child flag. All outputs required.
void* ComThinkConditionTree_FindInsertionPosition(void* tree, int* keyPtr, void** outNode, char* insertLeft, char* updateFirstNode) {
    char* p = (char*)tree;
    int zero = 0;
    char one = 1;
    *outNode = 0;
    void* current = p + 4;
    void* node = *(void**)(p + 4);
    *insertLeft = one;
    *updateFirstNode = one;
    while (node != 0) {
        int key = *keyPtr;
        current = node;
        int nodeKey = AsTreeStringNode8022C490(node)->key;
        if (key < nodeKey) {
            node = *(void**)node;
            *insertLeft = one;
        } else {
            *outNode = node;
            node = AsTreeStringNode8022C490(node)->right;
            *insertLeft = zero;
            *updateFirstNode = zero;
        }
    }
    return current;
}

// Return the same insertion outputs for the action tree; see the condition-tree contract.
void* ComThinkActionTree_FindInsertionPosition(void* tree, int* keyPtr, void** outNode, char* insertLeft, char* updateFirstNode) {
    char* p = (char*)tree;
    int zero = 0;
    char one = 1;
    *outNode = 0;
    void* current = p + 4;
    void* node = *(void**)(p + 4);
    *insertLeft = one;
    *updateFirstNode = one;
    while (node != 0) {
        int key = *keyPtr;
        current = node;
        int nodeKey = AsTreeStringNode8022C490(node)->key;
        if (key < nodeKey) {
            node = *(void**)node;
            *insertLeft = one;
        } else {
            *outNode = node;
            node = AsTreeStringNode8022C490(node)->right;
            *insertLeft = zero;
            *updateFirstNode = zero;
        }
    }
    return current;
}

// Cache the shared file buffer when it becomes available and latch readiness.
// loadedBuffer is borrowed from sharedFile. A latched fileReady is not cleared by
// RequestFileLoad here; callers must reset readiness before starting another load.
int CComThinkCtrl_PollFileLoad(void* self) {
    if (AsCommandState8022D190(self)->fileReady != 0) {
        return 1;
    }
    int result = CSharedFilePtr_GetLoadedBuffer(AsCommandState8022D190(self)->sharedFile);
    AsCommandState8022D190(self)->loadedBuffer = result;
    if (result == 0) {
        return 0;
    }
    AsCommandState8022D190(self)->fileReady = 1;
    return 1;
}

// Release an owned string buffer and optionally delete its containing object.
void* dtor_8022D684(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned int*)self >> 31) {
                appFreeUni__FPv(((D7String*)self)->data);
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Release an owned string buffer and optionally delete its containing object.
void* dtor_8022DA0C(void* self, int flag) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned int*)self >> 31) {
                appFreeUni__FPv(((D7String*)self)->data);
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Release the string embedded after the key word and optionally delete the record.
void* dtor_8022D74C(int self, int flag) {
    if (self != 0) {
        if (self + 4 != 0) {
          if (self + 4 != 0) {
            if (*(unsigned int*)(self + 4) >> 31) {
                appFreeUni__FPv(*(void**)(self + 0xC));
            }
          }
        }
        if (flag > 0) __dl__FPv((void*)self);
    }
    return (void*)self;
}

// Release the string embedded after the key word and optionally delete the record.
void* dtor_8022DAD4(int self, int flag) {
    if (self != 0) {
        if (self + 4 != 0) {
          if (self + 4 != 0) {
            if (*(unsigned int*)(self + 4) >> 31) {
                appFreeUni__FPv(*(void**)(self + 0xC));
            }
          }
        }
        if (flag > 0) __dl__FPv((void*)self);
    }
    return (void*)self;
}

} // extern "C"
