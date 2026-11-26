// BP object and loader teardown: borrowed views of the BP object and BP loader layouts,
// and the destructors that free string-value trees and the heap text stored in them.

#include "game/string.h"
#include "classes/CBpLoad.h"
#include "classes/CBpObject.h"
#include <decomp/cw_bitfields.h>
#include "classes/CSharedFileHandle.h"

// Borrow the BP object's reconstructed layout without transferring ownership.
inline CBpObject* AsCBpObject(void* object) {
    return (CBpObject*)object;
}

// Borrow the loader's reconstructed layout without transferring ownership.
inline CBpLoad* AsCBpLoad(void* object) {
    return (CBpLoad*)object;
}

#pragma use_lmw_stmw on

extern "C" {

struct TableLock;

struct VirtualOwned {
    virtual void destroy(int);
};

extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);

struct InstanceLock_u800C4568 {
    void* mutex;
    // Lock the borrowed per-loader mutex for the enclosing scope.
    InstanceLock_u800C4568(void* m) : mutex(m) {
        OSLockMutex(m);
    }
    // Release the per-loader mutex on both normal and exceptional exits.
    ~InstanceLock_u800C4568() {
        OSUnlockMutex(mutex);
    }
};

extern void TableLock_ctor(TableLock*, int, int);
extern void BpNamedGroupTree_ClearNodes(void*);

extern void* StringValueTree_dtor(void*, int);
extern void dtor_800C42A4(void*, int);
extern void dtor_800C3FE0(void*, int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void StringValueTree_DestroySubtree(void*, void*);

extern unsigned char __vt__9CBpObject[];
extern unsigned char __vt__7CBpLoad[];

struct FlagByte {
    CW_BITS2(unsigned char sign : 1, unsigned char low7 : 7)
    unsigned char firstInlineCharacter;
};

struct TableLock {
    // TableLock_ctor writes two words (8 bytes) into the guard.
    // Enter the shared BP-table lock with the original two lock-mode arguments.
    TableLock(int a, int b) {
        TableLock_ctor(this, a, b);
    }
    ~TableLock();
};

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;
    unsigned int flags;
    unsigned int heapStringLength;
    void* allocation;
};

struct StringValueTree {
    unsigned int count;
    TreeNode* root;
    TreeNode* first;
};
// The header is embedded: count at +0, root at +4, first at +8. Heap nodes
// have three links at +0/+4/+8 followed by a small/heap string at +0xC.
// Inline text belongs to the node; only strings with the high flag bit set
// own an additional allocation. These destructors free nodes and heap text.
enum StringStorage {
    kHeapStringBit = 31,
    kInlineStringLengthMask = 0x7f,
    kNodeStringOffset = (int)&((TreeNode*)0)->flags
};

// Release a node's heap-backed string before freeing the node itself.
inline void FreeStringValueNode(TreeNode* node) {
    if (((int)node + kNodeStringOffset) != 0 && ((int)node + kNodeStringOffset) != 0 &&
        ((node->flags >> kHeapStringBit) != 0))
        appFreeUni__FPv(node->allocation);
    appFreeUni__FPv(node);
}
// Recurse below the explicitly expanded levels, then release this subtree root.
inline void FreeStringValueLeaf(void* tree, TreeNode* node) {
    if (node->left != 0)
        StringValueTree_DestroySubtree(tree, node->left);
    if (node->right != 0)
        StringValueTree_DestroySubtree(tree, node->right);
    FreeStringValueNode(node);
}

// Destroy the named-string tree in postorder, then optionally free its container.
// self may be null; deleteFlag > 0 additionally frees self. Embedded headers
// use -1. Cached *Value/*Right/*Left aliases below are matching scaffolding.
// Traversal outline (L/R are child links):
//   visit LL, LR -> free L; visit RL, RR -> free R; free root.
// Each visit expands one more child level and delegates deeper recursion to
// StringValueTree_DestroySubtree, so every descendant dies before its parent.
void* StringValueTree_dtor(void* self, int deleteFlag) {
    TreeNode* root;
    TreeNode* leaf;
    TreeNode* branch;
    TreeNode* grandchild;
    // Keep the repeated base-destructor guards and cached reads: removing them changes code generation.
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            root = ((StringValueTree*)self)->root;
                            if (root) {
                                // Phase 1: release the entire left half, then its branch root.
                                TreeNode* leftValue = root->left;
                                branch = leftValue;
                                if (branch) {
                                    grandchild = branch->left;
                                    if (grandchild) {
                                        TreeNode* leftValue2 = grandchild->left;
                                        __typeof__(grandchild->right) cachedLeafRight1 = grandchild->right;
                                        leaf = leftValue2;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        TreeNode* rightValue = grandchild->right;
                                        leaf = rightValue;
                                        if (leaf != 0)
                                            FreeStringValueLeaf(self, leaf);
                                        FreeStringValueNode(grandchild);
                                    }
                                    TreeNode* rightValue2 = branch->right;
                                    TreeNode* rightValue3 = branch->right;
                                    __typeof__(branch->right) cachedBranchRight3 = branch->right;
                                    __typeof__(cachedBranchRight3) cachedBranchRight4 = branch->right;
                                    TreeNode* rightValue4 = branch->right;
                                    __typeof__(branch->right) cachedBranchRight6 = branch->right;
                                    TreeNode* rightValue5 = branch->right;
                                    TreeNode* rightValue6 = branch->right;
                                    TreeNode* rightValue7 = branch->right;
                                    TreeNode* rightValue8 = branch->right;
                                    __typeof__(branch->right) cachedBranchRight11 = branch->right;
                                    TreeNode* rightValue9 = branch->right;
                                    grandchild = rightValue9;
                                    if (grandchild) {
                                        __typeof__(grandchild->left) cachedLeafLeft1 = grandchild->left;
                                        TreeNode* leftValue3 = grandchild->left;
                                        __typeof__(grandchild->left) cachedLeafLeft3 = grandchild->left;
                                        __typeof__(cachedLeafLeft1) cachedLeafLeft4 = grandchild->left;
                                        TreeNode* leftValue4 = grandchild->left;
                                        TreeNode* leftValue5 = grandchild->left;
                                        TreeNode* leftValue6 = grandchild->left;
                                        leaf = leftValue3;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        FreeStringValueNode(grandchild);
                                    }
                                    FreeStringValueNode(branch);
                                }
                                // Phase 2: mirror the postorder walk for the right half.
                                branch = root->right;
                                if (0 != branch) {
                                    TreeNode* leftValue7 = branch->left;
                                    grandchild = leftValue7;
                                    if (grandchild) {
                                        __typeof__(grandchild->left) cachedLeafLeft8 = grandchild->left;
                                        TreeNode* rightValue10 = grandchild->right;
                                        TreeNode* leftValue8 = grandchild->left;
                                        TreeNode* rightValue11 = grandchild->right;
                                        leaf = leftValue8;
                                        __typeof__(grandchild->right) cachedLeafRight5 = grandchild->right;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        TreeNode* rightValue12 = grandchild->right;
                                        TreeNode* rightValue13 = grandchild->right;
                                        __typeof__(grandchild->right) cachedLeafRight8 = grandchild->right;
                                        TreeNode* rightValue14 = grandchild->right;
                                        leaf = rightValue13;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        FreeStringValueNode(grandchild);
                                    }
                                    TreeNode* rightValue15 = branch->right;
                                    TreeNode* rightValue16 = branch->right;
                                    __typeof__(branch->right) cachedBranchRight15 = branch->right;
                                    TreeNode* rightValue17 = branch->right;
                                    grandchild = rightValue17;
                                    if (grandchild) {
                                        TreeNode* leftValue9 = grandchild->left;
                                        leaf = leftValue9;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        leaf = grandchild->right;
                                        if (leaf)
                                            FreeStringValueLeaf(self, leaf);
                                        FreeStringValueNode(grandchild);
                                    }
                                    FreeStringValueNode(branch);
                                }
                                // Phase 3: both child subtrees are gone; release the root.
                                FreeStringValueNode(root);
                            }
                        }
                    }
                }
            }
        }
        if (0 < deleteFlag)
            __dl__FPv(self);
    }
    return self;
}

// Release scripts, object groups and loader handles under the table lock, then delete on request.
// Embedded owners within CBpObject (addresses, not the values at these fields):
// +18 objectCount -> object tree; +44 pad_044 -> secondary tree header;
// +50 namedGroupCount -> string-value tree; +60 scriptPathStorage -> NString.
// +24 objectArray and +5C loader are separately allocated polymorphic owners.
// ClearResources/ClearNodes run under the table lock. Header/string destruction
// follows outside it; deleteFlag > 0 finally frees the enclosing object.
CBpObject::~CBpObject() {
    *(void**)this = __vt__9CBpObject;
    {
        TableLock lock(1, 1);
        ClearResources();
        volatile char* objectBytes = (char*)this;
        BpNamedGroupTree_ClearNodes((void*)(objectBytes + (int)&((CBpObject*)0)->pad_044));
        {
            void* ownedObject = loader;
            if (ownedObject != 0) {
                ((VirtualOwned*)ownedObject)->destroy(1);
            }
        }
        {
            void* ownedObject = objectArray;
            if (ownedObject != 0) {
                ((VirtualOwned*)ownedObject)->destroy(1);
            }
        }
    }
    // All shared-table edits are complete; destroy each embedded owner once.
    ((NString*)scriptPathStorage)->~NString(); // CBpObject::~CBpObject: inline path at +0x60
    StringValueTree_dtor((char*)&namedGroupCount, -1);
    dtor_800C42A4(pad_044, -1);
    dtor_800C3FE0((char*)&objectCount, -1);
}

// Close an outstanding shared file, clear its filename under the mutex, and release filename storage.
// The file-close path is conditional on a NONEMPTY filename. selected length
// uses a pointer-shaped ABI word below but is only tested for zero, never read
// as an address. Emptying heap text retains its allocation until after unlock.
CBpLoad::~CBpLoad() {
    *(void**)this = __vt__7CBpLoad;
    {
        InstanceLock_u800C4568 lock((void*)((char*)&mutexStorage));
        unsigned int filenameFlags = filenameStorage;
        void* filenameLengthWord;
        if ((filenameFlags >> kHeapStringBit) == 0)
            filenameLengthWord = (void*)(*(unsigned char*)&filenameStorage & kInlineStringLengthMask);
        else
            filenameLengthWord = filenameLongLength;
        if (filenameLengthWord != 0) {
            void* ownedObject = sharedFileHandle;
            if (ownedObject != 0) {
                CSharedFileHandle::Destroy(ownedObject, 1);
                sharedFileHandle = 0;
            }
            if ((filenameStorage >> kHeapStringBit) == 0) {
                ((unsigned char*)&filenameStorage)[1] = 0;
                ((FlagByte*)((char*)&filenameStorage))->low7 = 0;
            } else {
                *(unsigned char*)filenameHeapData = 0;
                filenameLongLength = 0;
            }
        }
    }
    // The mutex scope ended; release heap text, then optionally the wrapper.
    if (((int)this + 8) != 0 && ((filenameStorage >> kHeapStringBit) != 0)) {
        appFreeUni__FPv(filenameHeapData);
    }
}

} // extern "C"
