// Attachment cache for the effect box: stores copies of spray and sequence attachment inputs
// in three trees owned by the CPvsFxBox singleton.
// Erasing an attachment updates the cached first-node link. Freeing the trees releases each leaf group.

#include "classes/CPvsFxBox.h"
#include "classes/CFxBox.h"

// The effect singleton owns a CFxBox after its two base-class subobjects.
struct FxObjectLayout {
    char sharedBaseStorage[0x18];
    CFxBox box;
};

// Borrow the existing object's shared layout; this view never owns storage.
static inline CPvsFxBox* AsCPvsFxBox(void* object) {
    return (CPvsFxBox*)object;
}

extern "C" {

// Attachment caches are ordered by the attachment object's address (32-bit key).
// Node links occupy +0..+8; parentColor low bit is color; key is +0xC.
// Payload follows at +0x10. Header sentinel is &header.root; count is not a node.
// Nodes own copies of input, and attachment->input is redirected to those copies.
// Erase/destruction frees cache nodes, not attachment objects or external resources.
// Input scalar units and word-block semantics are unresolved here. Cache edits
// must follow the producer records; these functions copy representations only.
struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode* parentColor;
    unsigned int key;
};

struct TreeHeader {
    unsigned int count;
    TreeNode* root;
};
struct FxCacheTrees {
    char reserved00[0x37C];
    TreeHeader spray;
    TreeNode* sprayFirst;
    TreeHeader sequence;
    TreeNode* sequenceFirst;
};
#define FX_OFFSET(type, field) ((int)&((type*)0)->field)
enum FxTreeOffsets {
    kSprayTree = FX_OFFSET(FxCacheTrees, spray),
    kSequenceTree = FX_OFFSET(FxCacheTrees, sequence)
};

// Find the insertion hint and predecessor for a key, reporting child side and whether no key <= the query was visited.
TreeNode* PvsFxCacheTree_FindInsertHint(TreeHeader* head, unsigned int* key, TreeNode** predecessorCandidate,
                                        unsigned char* left, unsigned char* noPredecessor) {
    // predecessorCandidate is the last visited key <= query (the predecessor candidate),
    // left selects the empty child link, and return is its parent or the sentinel.
    // noPredecessor stays 1 only if every visited key was greater than the query.
    // Equal keys take the right path too; callers perform a separate equality test.
    *predecessorCandidate = 0;
    TreeNode* root = head->root;
    *left = 1;
    TreeNode* insertionParent = (TreeNode*)&head->root;
    *noPredecessor = 1;
    TreeNode* node = root;
    while (node) {
        insertionParent = node;
        if (*key < node->key) {
            node = node->left;
            *left = 1;
        } else {
            *predecessorCandidate = node;
            node = node->right;
            *left = 0;
            *noPredecessor = 0;
        }
    }
    return insertionParent;
}

extern unsigned int g_fx;
void BaseObject_dtor(void*, int);
void __dl__FPv(void*);

// Clear the effect singleton, destroy its base and optionally delete the wrapper.
void* dtor_80102594(void* self, int freeFlag) {
    if (self != 0) {
        g_fx = 0;
        BaseObject_dtor(self, 0);
        if (freeFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

void RBTree_RebalanceAfterErase(TreeNode*, TreeNode*);
void appFreeUni__FPv(TreeNode*);
void FxTree_DestroySubtreeNodes(void*, void*);

struct TreeOwner {
    char pad[0x388];
    unsigned int count;
    TreeNode* head;
    TreeNode* firstNode;
};

// Erase a sequence attachment by pointer key and update the cached first-node link.
void CPvsFxBox::EraseSequenceAttachmentCache(unsigned int key) {
    TreeOwner* self = (TreeOwner*)this;
    TreeNode* node = self->head;
    TreeNode* candidate = (TreeNode*)&self->head;
    while (node != 0) {
        /* original 0x801023A8-0x801023C0: key >= target goes left (+0x0), else right (+0x4) */
        if (node->key >= key) {
            candidate = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (candidate != (TreeNode*)&self->head && key < candidate->key)
        candidate = (TreeNode*)&self->head;
    if (candidate != (TreeNode*)&self->head) {
        TreeNode* erased = candidate;
        if (candidate == self->firstNode) {
            TreeNode* right = candidate->right;
            if (right != 0) {
                do {
                    candidate = right;
                    right = right->left;
                } while (right != 0);
            } else {
                TreeNode* parent;
                do {
                    parent = (TreeNode*)((unsigned int)candidate->parentColor & ~1u);
                    /* original 0x80102434-0x80102448: climb while not the left child */
                    if (candidate == parent->left)
                        break;
                    candidate = parent;
                } while (1);
                candidate = parent;
            }
            self->firstNode = candidate;
        }
        RBTree_RebalanceAfterErase(erased, self->head);
        appFreeUni__FPv(erased);
        --self->count;
    }
}

struct CacheTree {
    char pad[0x37c];
    unsigned int count;
    TreeNode* root;
    TreeNode* firstNode;
};

// Erase a spray attachment by pointer key and update the cached first-node link.
void CPvsFxBox::EraseSprayAttachmentCache(unsigned int key) {
    CacheTree* self = (CacheTree*)this;
    TreeNode* erased;
    TreeNode* node = self->root;
    TreeNode* candidate = (TreeNode*)&self->root;
    while (node != 0) {
        if (node->key >= key) {
            candidate = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (candidate != (TreeNode*)&self->root && key < candidate->key)
        candidate = (TreeNode*)&self->root;
    if (candidate != (TreeNode*)&self->root) {
        erased = candidate;
        if (candidate == self->firstNode) {
            TreeNode* succ = candidate->right;
            if (succ != 0) {
                do {
                    candidate = succ;
                    succ = succ->left;
                } while (succ != 0);
            } else {
                TreeNode* parent;
                do {
                    parent = (TreeNode*)((unsigned int)candidate->parentColor & ~1u);
                    if (candidate == parent->left)
                        break;
                    candidate = parent;
                } while (1);
                candidate = parent;
            }
            self->firstNode = candidate;
        }
        RBTree_RebalanceAfterErase(erased, self->root);
        appFreeUni__FPv(erased);
        --self->count;
    }
}

extern unsigned int g_resourceManager;
extern float kFloat_33_333332_ShotEffects;
extern float kFloat_0_shotEffects;
extern char __vt__14CRenderFuncPtr[16];
extern char s_fxSingletonBaseVtable[16];
extern char __vt__3CFx[16];
extern char __vt__9CPvsFxBox[16];
extern char s_cameraTransform[16];
void BaseObject_ctor(void*);

struct BigObj {
    char pad[0x3C0];
};
TreeNode* FxBoxTree_FindInsertHint(void*, void*, TreeNode**, unsigned char*, unsigned char*);
TreeNode* RBTree_InsertNode_Large(void*, TreeNode*, unsigned char, unsigned char, void*);
void* memcpy(void*, const void*, unsigned int);
// Input cache: two float scalars followed by two 16-byte blocks. The blocks are
// copied back into the spray/sequence attachment after input is redirected to
// stable tree storage. Their numerical interpretation and scalar units remain
// unresolved; scalar0/scalar1 and the word blocks are not recovered matrix/vector types.
// Editing contract: copy all 40 bytes together. The two -1.0 scalar defaults are
// used only when lookup inserts a missing key; incoming scalars replace them.
// Each block is copied byte-for-byte, with no arithmetic or transpose. Changes to
// units or individual words require evidence from the attachment's consumer.
enum FxCacheCopySize128 {
    kFxCacheBlockWords = 4,
    kFxCacheBlockCount = 2,
    kFxCacheInputBytes = 40
};
struct CacheData {
    unsigned int key;
    float scalar0, scalar1;
    unsigned int data[kFxCacheBlockWords * kFxCacheBlockCount];
};
struct InputData {
    float scalar0, scalar1;
    unsigned int data[kFxCacheBlockWords * kFxCacheBlockCount];
};
typedef char FxCacheInputSize128[(sizeof(InputData) == kFxCacheInputBytes) ? 1 : -1];
// This cache is a lifetime bridge, not a tuning layer: mutate the incoming
// attachment input before caching. The cache redirects its input pointer to an
// owned 40-byte copy, then mirrors the two opaque word blocks back to the entry.
// No unit conversion, interpolation or interpretation of those blocks occurs here.
extern float kFloat_Neg1_0;
#pragma push
#pragma opt_common_subs off
struct StoredInput {
    float scalar0, scalar1;
    unsigned int firstWords[kFxCacheBlockWords], secondWords[kFxCacheBlockWords]; // first/second opaque cache blocks
    // Initialize unresolved scalar values to -1 and both cached word blocks to zero.
    StoredInput() {
        scalar0 = kFloat_Neg1_0;
        scalar1 = kFloat_Neg1_0;
        for (int i = 0; i < 4; ++i) {
            firstWords[i] = 0;
            secondWords[i] = 0;
        }
    }
};
// Cache payloads are copied as two scalars plus two four-word blocks.
struct SprayAttachmentEntry {
    char reserved00[0x14];
    StoredInput* input;
    char reserved18[0x14];
    unsigned int firstBlock[4], secondBlock[4];
};
struct SequenceAttachmentEntry {
    char reserved00[8];
    void* input;
    char reserved0C[8];
    unsigned int firstBlock[4], secondBlock[4];
};
struct FxCacheNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode* parentColor;
    unsigned int key;
    StoredInput value;
};
struct StoredPair {
    unsigned int key;
    StoredInput value;
    // Copy the address key and input value into a cache payload.
    StoredPair(unsigned int k, const StoredInput& v) : key(k), value(v) {}
};
// Find or create a spray cache record initialized with sentinel scalars and empty word blocks.
inline StoredPair* cacheLookup(CacheTree* self, unsigned int& key) {
    TreeNode* predecessorCandidate;
    unsigned char left, noPredecessor;
    TreeNode* node = PvsFxCacheTree_FindInsertHint((TreeHeader*)((char*)self + kSprayTree), &key, &predecessorCandidate,
                                                   &left, &noPredecessor);
    StoredPair* result;
    if (predecessorCandidate == 0 || (result = (StoredPair*)((char*)&predecessorCandidate->key), result->key < key)) {
        StoredPair value(key, StoredInput());
        result =
            (StoredPair*)((char*)RBTree_InsertNode_Large((char*)self + kSprayTree, node, left, noPredecessor, &value) +
                          FX_OFFSET(TreeNode, key));
    }
    return result;
}
// Copy spray input into stable tree storage and refresh the attachment's two cached word blocks.
void CPvsFxBox::CacheSprayAttachmentInput(void* entry) {
    CacheTree* self = (CacheTree*)this;
    // Snapshot the borrowed input before looking up or creating address-keyed cache storage.
    if ((*(void**)&((SprayAttachmentEntry*)entry)->input) != 0) {
        StoredInput* input = ((SprayAttachmentEntry*)entry)->input;
        StoredPair key((unsigned int)entry, *input);
        TreeNode* predecessorCandidate;
        unsigned char left, noPredecessor;
        TreeNode* node =
            FxBoxTree_FindInsertHint((char*)self + kSprayTree, &key, &predecessorCandidate, &left, &noPredecessor);
        if (predecessorCandidate == 0 || predecessorCandidate->key < key.key)
            RBTree_InsertNode_Large((char*)self + kSprayTree, node, left, noPredecessor, &key);
        (*(void**)&((SprayAttachmentEntry*)entry)->input) = &cacheLookup(self, (unsigned int&)entry)->value;
        memcpy(((SprayAttachmentEntry*)entry)->firstBlock, cacheLookup(self, (unsigned int&)entry)->value.firstWords,
               0x10);
        memcpy(((SprayAttachmentEntry*)entry)->secondBlock, cacheLookup(self, (unsigned int&)entry)->value.secondWords,
               0x10);
    }
}

#pragma pop
enum CacheInsertFlagByte {
    kInsertNoPredecessorByte = 0,
    kInsertLeftChildByte = 1
};
// Borrow either byte of a two-byte scalar object; both outputs are within declared storage.
static inline unsigned char& CacheInsertFlag(unsigned short& storage, CacheInsertFlagByte which) {
    return ((unsigned char*)&storage)[which];
}
typedef char CacheInsertFlagStorageSize[(sizeof(unsigned short) == 2) ? 1 : -1];

struct CacheSearch {
    unsigned int key;
    float scalar0, scalar1;
    unsigned int data[8];
};
struct CacheValue {
    unsigned int key;
    StoredInput value;
};
#pragma push
#pragma opt_common_subs off
// Copy sequence input into stable tree storage and refresh the attachment's two cached word blocks.
void CPvsFxBox::CacheSequenceAttachmentInput(void* entry) {
    BigObj* self = (BigObj*)this;
    // Snapshot the borrowed input before looking up or creating address-keyed cache storage.
    void* data = ((SequenceAttachmentEntry*)entry)->input;
    if (data != 0) {
        // Phase 1: snapshot caller input before redirecting the attachment pointer.
        CacheSearch search;
        search.key = (unsigned int)entry;
        search.scalar0 = *(float*)data;
        search.scalar1 = *((float*)data + 1);
        for (int i = 0; i < 8; ++i)
            search.data[i] = ((unsigned int*)data)[2 + i];
        TreeNode* found;
        // Two byte outputs share one scalar object. The former one-byte local
        // overflowed at +1; unsigned short keeps the same 0x0E/0x0F stack slots
        // while declaring BOTH bytes. No numerical short load or endian shift.
        unsigned short insertionFlagBytes;
        TreeNode* node = FxBoxTree_FindInsertHint((char*)self + kSequenceTree, &search, &found,
                                                  &CacheInsertFlag(insertionFlagBytes, kInsertLeftChildByte),
                                                  &CacheInsertFlag(insertionFlagBytes, kInsertNoPredecessorByte));
        if (found == 0 || found->key < (unsigned int)entry)
            RBTree_InsertNode_Large((char*)self + kSequenceTree, node,
                                    CacheInsertFlag(insertionFlagBytes, kInsertLeftChildByte),
                                    CacheInsertFlag(insertionFlagBytes, kInsertNoPredecessorByte), &search);
        TreeNode* predecessorCandidate;
        unsigned char leftChild, noPredecessor;
        node = PvsFxCacheTree_FindInsertHint((TreeHeader*)((char*)self + kSequenceTree), (unsigned int*)&entry,
                                             &predecessorCandidate, &leftChild, &noPredecessor);
        if (predecessorCandidate == 0 || predecessorCandidate->key < (unsigned int)entry) {
            CacheValue value = {(unsigned int)entry, StoredInput()};
            node = RBTree_InsertNode_Large((char*)self + kSequenceTree, node, leftChild, noPredecessor, &value);
        } else {
            node = predecessorCandidate;
            /* original 0x80102144/0x80102154 (found: r6 = predecessorCandidate+0xc, used at 0x801021D8): an existing entry is used from the found node, not the insertion hint */
        }
        // Phase 2: redirect to node-owned input; erasing that cache invalidates this pointer.
        ((SequenceAttachmentEntry*)entry)->input = &((FxCacheNode*)node)->value;
        node = PvsFxCacheTree_FindInsertHint((TreeHeader*)((char*)self + kSequenceTree), (unsigned int*)&entry,
                                             &predecessorCandidate, &leftChild, &noPredecessor);
        if (predecessorCandidate == 0 || predecessorCandidate->key < (unsigned int)entry) {
            CacheValue value = {(unsigned int)entry, StoredInput()};
            node = RBTree_InsertNode_Large((char*)self + kSequenceTree, node, leftChild, noPredecessor, &value);
        } else {
            node = predecessorCandidate;
            /* original 0x80102204/0x80102214 (found: r5 = predecessorCandidate+0xc, used at 0x80102298): an existing entry is used from the found node, not the insertion hint */
        }
        // Phase 3: refresh both attachment blocks from the cached copy.
        memcpy(((SequenceAttachmentEntry*)entry)->firstBlock, ((FxCacheNode*)node)->value.firstWords, 0x10);
        /* original 0x80102290-0x801022A4: r5 = node+0xc, r4 = r5+0xc, the stored input's first opaque 16-byte block */
        node = PvsFxCacheTree_FindInsertHint((TreeHeader*)((char*)self + kSequenceTree), (unsigned int*)&entry,
                                             &predecessorCandidate, &leftChild, &noPredecessor);
        if (predecessorCandidate == 0 || predecessorCandidate->key < (unsigned int)entry) {
            CacheValue value = {(unsigned int)entry, StoredInput()};
            node = RBTree_InsertNode_Large((char*)self + kSequenceTree, node, leftChild, noPredecessor, &value);
        } else {
            node = predecessorCandidate;
            /* original 0x801022CC/0x801022DC (found: r5 = predecessorCandidate+0xc, used at 0x80102360): an existing entry is used from the found node, not the insertion hint */
        }
        memcpy(((SequenceAttachmentEntry*)entry)->secondBlock, ((FxCacheNode*)node)->value.secondWords, 0x10);
        /* original 0x80102358-0x8010236C: r4 = (node+0xc)+0x1c, the second opaque 16-byte block */
    }
}
#pragma pop

// Construct the effect singleton and embedded box, initialize its three trees and bind resource/camera providers.
CPvsFxBox::CPvsFxBox() {
    BigObj* self = (BigObj*)this;
    char* effectBase = (char*)self;
    char* singletonBase = effectBase + 4;
    float f0;
    float stepMilliseconds;
    *(void**)effectBase = __vt__14CRenderFuncPtr;
    BaseObject_ctor(singletonBase);
    *(void**)singletonBase = s_fxSingletonBaseVtable;
    if (singletonBase != 0)
        singletonBase -= 4;
    g_fx = (unsigned int)singletonBase;
    char* embeddedBox = effectBase + 0x18;
    *(void**)effectBase = __vt__3CFx;
    (*(void**)&AsCPvsFxBox(effectBase)->unk_4) = __vt__3CFx + 12;
    ((CFxBox*)embeddedBox)->__ct();
    *(void**)embeddedBox = __vt__9CPvsFxBox;
    *(unsigned int*)(embeddedBox + FX_OFFSET(FxCacheTrees, spray.count)) = 0;
    *(unsigned int*)(embeddedBox + FX_OFFSET(FxCacheTrees, spray.root)) = 0;
    *(void**)(embeddedBox + FX_OFFSET(FxCacheTrees, sprayFirst)) = embeddedBox + FX_OFFSET(FxCacheTrees, spray.root);
    *(unsigned int*)(embeddedBox + FX_OFFSET(FxCacheTrees, sequence.count)) = 0;
    *(unsigned int*)(embeddedBox + FX_OFFSET(FxCacheTrees, sequence.root)) = 0;
    *(void**)(embeddedBox + FX_OFFSET(FxCacheTrees, sequenceFirst)) =
        embeddedBox + FX_OFFSET(FxCacheTrees, sequence.root);
    AsCPvsFxBox(effectBase)->auxiliaryAttachmentTreeCount = 0;
    AsCPvsFxBox(effectBase)->auxiliaryAttachmentTreeRoot = 0;
    AsCPvsFxBox(effectBase)->auxiliaryAttachmentTreeRightmost =
        effectBase + FX_OFFSET(CPvsFxBox, auxiliaryAttachmentTreeRoot);
    ((CFxBox*)embeddedBox)->SetResourceManager((void*)g_resourceManager);
    ((CFxBox*)embeddedBox)->SetCameraTransform(s_cameraTransform);
    stepMilliseconds = kFloat_33_333332_ShotEffects;
    int enabled = 1;
    f0 = kFloat_0_shotEffects;
    ((FxObjectLayout*)effectBase)->box.nominalStepMilliseconds = stepMilliseconds;
    // Wrapper state starts as bytes {1,0,1} and scalar +0x14 as 0.0.
    // No behavior has been established for these fields in this unit; they are
    // independent of the embedded box's nominalStepMilliseconds above. Keep placeholders
    // until a consuming method establishes what toggling each byte does.
    AsCPvsFxBox(effectBase)->unk_3B8 = (char)enabled;
    AsCPvsFxBox(effectBase)->unk_3B9 = 0;
    AsCPvsFxBox(effectBase)->unk_3BA = (char)enabled;
    AsCPvsFxBox(effectBase)->unk_14 = f0;
}

#pragma push
#pragma use_lmw_stmw on
// Release a leaf group after its descendants; node and header are borrowed.
static inline void FreeFxCacheLeafGroup(void* tree, TreeNode*& node) {
    if (node != 0) {
        if (node->left != 0)
            FxTree_DestroySubtreeNodes(tree, node->left);
        if (node->right != 0)
            FxTree_DestroySubtreeNodes(tree, node->right);
        appFreeUni__FPv(node);
    }
}

// Free both leaf groups before the child; leftGroup starts at child->left.
// Reference temporaries keep the caller's matched link lifetimes and register allocation.
static inline void FreeFxCacheChildGroup(void* tree, TreeNode*& child, TreeNode*& leftGroup) {
    FreeFxCacheLeafGroup(tree, leftGroup);
    leftGroup = child->right;
    FreeFxCacheLeafGroup(tree, leftGroup);
    appFreeUni__FPv(child);
}

// Free header-owned nodes in left/right/node postorder, then optionally delete the header.
// Destroy the attachment-input cache and optionally release its header allocation.
void* dtor_80102B54(void* self, int freeFlag) {
    if (0 != self) {
        // Five redundant guards are real matched branches, not extra tree levels.
        // Seven link snapshots carry no ownership; their declaration order preserves register lifetimes.
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            TreeNode* root = ((TreeHeader*)self)->root;
                            // original +0x4: tree header, not CPvsFxBox
                            if (root != 0) {
                                TreeNode* branch = root->left;
                                if (0 != branch) {
                                    TreeNode* child = branch->left;
                                    if (child != 0) {
                                        TreeNode* leftLeafSnapshot = child->left;
                                        TreeNode* grandchild = leftLeafSnapshot;
                                        FreeFxCacheChildGroup(self, child, grandchild);
                                    }
                                    TreeNode* rightChildSnapshot = branch->right;
                                    child = rightChildSnapshot;
                                    if (child) {
                                        TreeNode* leftLeafSnapshot = child->left;
                                        TreeNode* grandchild = leftLeafSnapshot;
                                        FreeFxCacheChildGroup(self, child, grandchild);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                TreeNode* rightBranchSnapshot = root->right;
                                branch = rightBranchSnapshot;
                                if (branch != 0) {
                                    TreeNode* leftChildSnapshot = branch->left;
                                    TreeNode* child = leftChildSnapshot;
                                    if (0 != child) {
                                        TreeNode* leftLeafSnapshot = child->left;
                                        TreeNode* grandchild = leftLeafSnapshot;
                                        FreeFxCacheChildGroup(self, child, grandchild);
                                    }
                                    child = branch->right;
                                    if (0 != child) {
                                        TreeNode* leftLeafSnapshot = child->left;
                                        TreeNode* grandchild = leftLeafSnapshot;
                                        FreeFxCacheChildGroup(self, child, grandchild);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        if (0 < freeFlag)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

void FxBoxResourceTree_FreeSubtreeNodes(void*, void*);
void __dt__6CFxBoxFv(void*, int);

// Free one leaf group; descendants use the existing subtree destructor.
inline void FreeFxResourceLeaf(TreeNode* node, void* tree) {
    TreeNode* tmp = node;
    if (tmp != 0) {
        if (tmp->left != 0)
            FxBoxResourceTree_FreeSubtreeNodes(tree, (void*)tmp->left);
        if (tmp->right != 0)
            FxBoxResourceTree_FreeSubtreeNodes(tree, (void*)tmp->right);
        appFreeUni__FPv(tmp);
    }
}

// Free both attachment caches, destroy the embedded effect box and, when asked,
// the wrapper itself. A subtree is released in left/right/node order, so nodes go
// before the node that points at them.
//
// Common subexpression folding is off because the original recomputes a tree
// header's address for every free instead of holding it in a register; the
// repeated tests on that address are part of the same shape.
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
CPvsFxBox::~CPvsFxBox() {
    // Phase 1: sequence attachment copies, including their input payloads.
    // Both trees use a four-level postorder expansion. Leaf helpers recurse below
    // that boundary; the remaining link snapshots preserve the required allocation.
    // Deeper resource nodes are released by FxBoxResourceTree_FreeSubtreeNodes.
    if ((char*)this + kSequenceTree) {
        if ((char*)this + kSequenceTree) {
            if ((char*)this + kSequenceTree) {
                if ((char*)this + kSequenceTree) {
                    if ((char*)this + kSequenceTree) {
                        if ((char*)this + kSequenceTree) {
                            TreeNode* root = (TreeNode*)sequenceAttachmentTreeRoot;
                            if (root != 0) {
                                TreeNode* branch = root->left;
                                if (branch) {
                                    TreeNode* child = branch->left;
                                    if (child) {
                                        TreeNode* leftChildSnapshot = child->left;
                                        TreeNode* grandchild = leftChildSnapshot;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        appFreeUni__FPv(child);
                                    }
                                    TreeNode* rightChildSnapshot = branch->right;
                                    child = rightChildSnapshot;
                                    if (child) {
                                        TreeNode* leftChildReload = child->left;
                                        TreeNode* grandchild = leftChildReload;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        appFreeUni__FPv(child);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                TreeNode* rightChildSnapshot = root->right;
                                branch = rightChildSnapshot;
                                if (branch) {
                                    TreeNode* leftChildSnapshot = branch->left;
                                    TreeNode* child = leftChildSnapshot;
                                    if (child) {
                                        TreeNode* grandchild = child->left;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        TreeNode* rightChildReloadAgain = child->right;
                                        grandchild = rightChildReloadAgain;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        appFreeUni__FPv(child);
                                    }
                                    child = branch->right;
                                    if (child) {
                                        TreeNode* leftChildSnapshot = child->left;
                                        TreeNode* grandchild = leftChildSnapshot;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSequenceTree);
                                        appFreeUni__FPv(child);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
    }
    // Phase 2: spray attachment copies; the effect box is still alive.
    if ((char*)this + kSprayTree) {
        if ((char*)this + kSprayTree) {
            if ((char*)this + kSprayTree) {
                if ((char*)this + kSprayTree) {
                    if ((char*)this + kSprayTree) {
                        if ((char*)this + kSprayTree) {
                            TreeNode* root = (TreeNode*)sprayAttachmentTreeRoot;
                            if (root) {
                                TreeNode* branch = root->left;
                                if (branch) {
                                    TreeNode* leftChildSnapshot = branch->left;
                                    TreeNode* child = leftChildSnapshot;
                                    if (child) {
                                        TreeNode* grandchild = child->left;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        appFreeUni__FPv(child);
                                    }
                                    child = branch->right;
                                    if (child) {
                                        TreeNode* grandchild = child->left;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        appFreeUni__FPv(child);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                TreeNode* rightChildSnapshot = root->right;
                                branch = rightChildSnapshot;
                                if (branch) {
                                    TreeNode* leftChildReload = branch->left;
                                    TreeNode* child = leftChildReload;
                                    if (0 != child) {
                                        TreeNode* leftChildSnapshot = child->left;
                                        TreeNode* leftChildAlias = leftChildSnapshot;
                                        TreeNode* leftChildAliasAgain = leftChildAlias;
                                        TreeNode* grandchild = child->left;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        appFreeUni__FPv(child);
                                    }
                                    child = branch->right;
                                    if (child) {
                                        TreeNode* grandchild = child->left;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        grandchild = child->right;
                                        FreeFxResourceLeaf(grandchild, (char*)this + kSprayTree);
                                        appFreeUni__FPv(child);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
    }
    // Phase 3: destroy the embedded effect box. The generated destructor frees the
    // wrapper when the caller asked for it.
    __dt__6CFxBoxFv(this, 0);
}
#pragma pop

} // extern "C"

// Private views must retain the original 32-bit ABI offsets.
typedef char u80101BD0_layout_check_0[(FX_OFFSET(FxCacheNode, value.firstWords) == 0x18) ? 1 : -1];
typedef char u80101BD0_layout_check_1[(FX_OFFSET(FxCacheNode, value.secondWords) == 0x28) ? 1 : -1];
typedef char u80101BD0_layout_check_2[(FX_OFFSET(SprayAttachmentEntry, firstBlock) == 0x2C) ? 1 : -1];
typedef char u80101BD0_layout_check_3[(FX_OFFSET(SequenceAttachmentEntry, firstBlock) == 0x14) ? 1 : -1];
