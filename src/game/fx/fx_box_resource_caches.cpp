// The effect box's resource caches. Each cache is a tree of nodes that own a
// string key and a cached object, and clearing one releases the objects first,
// then the keys and the node storage, leaving an empty tree behind.
#include "classes/CFxBox.h"

extern "C" {

void FxBoxResourceMap_dtor(void*, int);
void FxSequenceTemplateMap_dtor(void*, int);
void FxSprayTemplateMap_dtor(void*, int);
void FxSequenceTree_FreeSubtree(void*, void*);
void FxBoxResourceMap_DestroySubtree(void*, void*);
void StringValueTree_DestroySubtree(void*, void*);
void Object_DeleteVirtual(void*, void*);
void ResourceManager_ReleaseId(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
extern char s_fxBoxVtable[];

} // extern "C"

// Tree contract: child[0]/[1] are left/right; parent bit 0 is colour, not address.
// Cache nodes carry a string key (owns bit 31 => value must be freed), then cachedObject/object.
// Each cache stores count, root and first; the address of root is the end sentinel.
// Empty cache: count=0, root=0, first=&root. Payload destruction precedes node/key destruction.
struct FxTreeNode {
    FxTreeNode* child[2];
    FxTreeNode* parent;
    unsigned int owns;
    char stringLength[4];
    void* value;
    void* cachedObject;
    void* object;
};

// Prefix of an effect registry tree: a node count and the root link.
struct FxTreeOwner {
    void* nodeCount;
    FxTreeNode* root;
};

// True when the node's string key is heap-allocated (bit 31 of `owns`) and must be freed with the node.
// The `&owns` operand is always non-null; it stays because removing it changes the generated code.
#define FXNODE_FIELD_OFFSET(field) ((int)&((FxTreeNode*)0)->field)
#define KEY_IS_HEAP(n) ((char*)(n) + FXNODE_FIELD_OFFSET(owns) && (((unsigned int)(n)->owns) >> 31))

// Unrolling map: BRANCH -> PAIR -> LEAF -> external recursive destructor.
// Registry leaves free nodes only; cache leaves also free owned string keys.
// Outer unrolling levels stay macros to retain the original register allocation.
// Their owner is explicit; node arguments must be stable lvalues with no side effects.
// Free both deeper registry subtrees and the non-null node; does not destroy a cache payload/key.
static inline void FreeSequenceLeaf(void* self, FxTreeNode*& n) {
    if ((n)->child[0])
        FxSequenceTree_FreeSubtree(self, (n)->child[0]);
    if ((n)->child[1])
        FxSequenceTree_FreeSubtree(self, (n)->child[1]);
    appFreeUni__FPv(n);
}

#define FREE_SEQUENCE_PAIR(owner, n)                \
    do {                                            \
        if (n) {                                    \
            FxTreeNode* pairLeft = (n)->child[0];   \
            if (pairLeft)                           \
                FreeSequenceLeaf(owner, pairLeft);  \
            FxTreeNode* pairRight = (n)->child[1];  \
            if (pairRight)                          \
                FreeSequenceLeaf(owner, pairRight); \
            appFreeUni__FPv(n);                     \
        }                                           \
    } while (0)

#define FREE_SEQUENCE_BRANCH(owner, n)             \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* treeLeft = (n)->child[0];  \
            FREE_SEQUENCE_PAIR(owner, treeLeft);   \
            FxTreeNode* treeRight = (n)->child[1]; \
            FREE_SEQUENCE_PAIR(owner, treeRight);  \
            appFreeUni__FPv(n);                    \
        }                                          \
    } while (0)

// Free deeper sprite-cache subtrees, this node's owned key, then the node (null accepted).
static inline void FreeResourceLeaf(CFxBox* box, FxTreeNode*& n) {
    if (n) {
        void* eecItem = (void*)(n)->child[0];
        if (eecItem)
            FxBoxResourceMap_DestroySubtree(&box->spriteCacheCount, eecItem);
        eecItem = (void*)(n)->child[1];
        if (eecItem)
            FxBoxResourceMap_DestroySubtree(&box->spriteCacheCount, eecItem);
        if (KEY_IS_HEAP(n))
            appFreeUni__FPv((n)->value);
        appFreeUni__FPv(n);
    }
}
#define FREE_RESOURCE_PAIR(owner, n)              \
    do {                                          \
        if (n) {                                  \
            FxTreeNode* eecLeft = (n)->child[0];  \
            FreeResourceLeaf(owner, eecLeft);     \
            FxTreeNode* eecRight = (n)->child[1]; \
            FreeResourceLeaf(owner, eecRight);    \
            if (KEY_IS_HEAP(n))                   \
                appFreeUni__FPv((n)->value);      \
            appFreeUni__FPv(n);                   \
        }                                         \
    } while (0)
#define FREE_RESOURCE_BRANCH(owner, n)             \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* treeLeft = (n)->child[0];  \
            FREE_RESOURCE_PAIR(owner, treeLeft);   \
            FxTreeNode* treeRight = (n)->child[1]; \
            FREE_RESOURCE_PAIR(owner, treeRight);  \
            if (KEY_IS_HEAP(n))                    \
                appFreeUni__FPv((n)->value);       \
            appFreeUni__FPv(n);                    \
        }                                          \
    } while (0)

// A cached spray template; only its virtual destructor is reached here.
struct SprayTemplateObject {
    virtual ~SprayTemplateObject();
};

// Free an owned spray-template string key and its non-null node; payload deletion precedes this call.
static inline void FreeSprayNodeValue(FxTreeNode*& n) {
    if ((char*)(n) + FXNODE_FIELD_OFFSET(owns)) {
        if (KEY_IS_HEAP(n))
            appFreeUni__FPv((n)->value);
    }
    appFreeUni__FPv(n);
}
#define FREE_SPRAY_LEAF(owner, n)                                                      \
    do {                                                                               \
        if (n) {                                                                       \
            void* item = (void*)(n)->child[0];                                         \
            if (item)                                                                  \
                StringValueTree_DestroySubtree(&owner->sprayTemplateCacheCount, item); \
            item = (void*)(n)->child[1];                                               \
            if (item)                                                                  \
                StringValueTree_DestroySubtree(&owner->sprayTemplateCacheCount, item); \
            FreeSprayNodeValue(n);                                                     \
        }                                                                              \
    } while (0)
#define FREE_SPRAY_PAIR(owner, n)                  \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* pairLeft = (n)->child[0];  \
            FREE_SPRAY_LEAF(owner, pairLeft);      \
            FxTreeNode* pairRight = (n)->child[1]; \
            FREE_SPRAY_LEAF(owner, pairRight);     \
            FreeSprayNodeValue(n);                 \
        }                                          \
    } while (0)
#define FREE_SPRAY_BRANCH(owner, n)                \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* treeLeft = (n)->child[0];  \
            FREE_SPRAY_PAIR(owner, treeLeft);      \
            FxTreeNode* treeRight = (n)->child[1]; \
            FREE_SPRAY_PAIR(owner, treeRight);     \
            FreeSprayNodeValue(n);                 \
        }                                          \
    } while (0)

// Advance to the in-order successor, masking the tree-colour bit from parent pointers.
// (Same routine as FxTreeNext_Sequence below; kept as two copies, one per function.)
inline void FxTreeNext_Spray(FxTreeNode*& node) {
    FxTreeNode* next = node->child[1];
    if (next) {
        while (next->child[0])
            next = next->child[0];
        node = next;
        return;
    } else {
        FxTreeNode* parent;
        while (node != (parent = (FxTreeNode*)((unsigned int)node->parent & ~1u))->child[0]) {
            node = parent;
        }
        node = parent;
    }
}

// Free an owned sequence-template string key and its non-null node; children were already released.
static inline void FreeTemplateNodeValue(FxTreeNode*& n) {
    if ((char*)(n) + FXNODE_FIELD_OFFSET(owns)) {
        if ((char*)(n) + FXNODE_FIELD_OFFSET(owns) && ((n)->owns >> 31))
            appFreeUni__FPv((n)->value);
    }
    appFreeUni__FPv(n);
}
#define FREE_TEMPLATE_LEAF(owner, n)                                                      \
    do {                                                                                  \
        if (n) {                                                                          \
            void* item = (void*)(n)->child[0];                                            \
            if (item)                                                                     \
                StringValueTree_DestroySubtree(&owner->sequenceTemplateCacheCount, item); \
            item = (void*)(n)->child[1];                                                  \
            if (item)                                                                     \
                StringValueTree_DestroySubtree(&owner->sequenceTemplateCacheCount, item); \
            FreeTemplateNodeValue(n);                                                     \
        }                                                                                 \
    } while (0)
#define FREE_TEMPLATE_PAIR(owner, n)               \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* pairLeft = (n)->child[0];  \
            FREE_TEMPLATE_LEAF(owner, pairLeft);   \
            FxTreeNode* pairRight = (n)->child[1]; \
            FREE_TEMPLATE_LEAF(owner, pairRight);  \
            FreeTemplateNodeValue(n);              \
        }                                          \
    } while (0)
#define FREE_TEMPLATE_BRANCH(owner, n)             \
    do {                                           \
        if (n) {                                   \
            FxTreeNode* treeLeft = (n)->child[0];  \
            FREE_TEMPLATE_PAIR(owner, treeLeft);   \
            FxTreeNode* treeRight = (n)->child[1]; \
            FREE_TEMPLATE_PAIR(owner, treeRight);  \
            FreeTemplateNodeValue(n);              \
        }                                          \
    } while (0)

// A cached sequence template; only its virtual destructor is reached here.
struct SequenceTemplateObject {
    virtual ~SequenceTemplateObject();
};

// Advance to the in-order successor, masking the tree-colour bit from parent pointers.
inline void FxTreeNext_Sequence(FxTreeNode*& node) {
    FxTreeNode* next = node->child[1];
    if (next) {
        while (next->child[0])
            next = next->child[0];
        node = next;
        return;
    } else {
        FxTreeNode* parent;
        while (node != (parent = (FxTreeNode*)((unsigned int)node->parent & ~1u))->child[0]) {
            node = parent;
        }
        node = parent;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Release the spray registry nodes and optionally delete the registry storage.
extern "C" void* FxSprayTree_dtor(FxTreeNode* self, int flags) {
    // The stacked `if (self)` guards are inlined base-class destructors of the tree container;
    // in plain C this is: free every node of the tree, then delete self if flags > 0.
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            FxTreeNode* root = ((FxTreeOwner*)self)->root;
                            if (root) {
                                FxTreeNode* a = root->child[0];
                                FREE_SEQUENCE_BRANCH(self, a);
                                FxTreeNode* b = root->child[1];
                                FREE_SEQUENCE_BRANCH(self, b);
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
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release the effect-sequence registry nodes and optionally delete the registry storage.
extern "C" FxTreeNode* FxSequenceTree_dtor(FxTreeNode* self, int flags) {
    // Same shape as FxSprayTree_dtor: free every node of the tree, then delete self if flags > 0.
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            FxTreeNode* root = ((FxTreeOwner*)self)->root;
                            if (root) {
                                FxTreeNode* a = root->child[0];
                                FREE_SEQUENCE_BRANCH(self, a);
                                FxTreeNode* b = root->child[1];
                                FREE_SEQUENCE_BRANCH(self, b);
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
#pragma pop

// Reset resources, destroy each effect registry, and optionally delete the effect box.
// Kept out of the class: its owners call the deleting form directly instead of
// placing the box on the stack.
CFxBox::~CFxBox() {
    vtbl = s_fxBoxVtable;
    ResetResources();
    FxSequenceTree_dtor((FxTreeNode*)&sequenceCount, -1);
    FxSprayTree_dtor((FxTreeNode*)&sprayCount, -1);
    FxBoxResourceMap_dtor(&spriteCacheCount, -1);
    FxSequenceTemplateMap_dtor(&sequenceTemplateCacheCount, -1);
    FxSprayTemplateMap_dtor(&sprayTemplateCacheCount, -1);
}

// Clear live effects before releasing sprite, spray-template, and sequence-template caches.
void CFxBox::ResetResources() {
    ClearEffects(1);
    ClearCachedResourceMap();
    ClearSprayTemplateCache();
    ClearSequenceTemplateCache();
}

// Store a borrowed resource-manager address; no retain/release.
void CFxBox::SetResourceManager(void* manager) {
    resourceManager = manager;
}

// Store a borrowed camera-transform address; no copy is made.
void CFxBox::SetCameraTransform(const void* transform) {
    cameraTransformAddress = transform;
}

#pragma push
#pragma use_lmw_stmw on
// Release cached objects and resource IDs in key order, then free the owned string-key nodes.
void CFxBox::ClearCachedResourceMap() {
    FxTreeNode* sentinel = (FxTreeNode*)&spriteCacheRoot;
    FxTreeNode* node = (FxTreeNode*)spriteCacheFirst;
    // Phase 1: release both payload handles while the tree links still exist.
    while (node != sentinel) {
        void* owner = resourceManager;
        if (owner) {
            void* first = node->cachedObject;
            if (first) {
                Object_DeleteVirtual(owner, first);
                node->cachedObject = 0;
            }
            void* second = node->object;
            if (second) {
                ResourceManager_ReleaseId(owner, second);
                node->object = 0;
            }
        }
        // In-order successor: descend right/left, otherwise climb untagged parents.
        FxTreeNode* next = node->child[1];
        if (next) {
            node = next;
            while (node->child[0])
                node = node->child[0];
        } else {
            FxTreeNode* parent;
            do {
                parent = (FxTreeNode*)((unsigned int)node->parent & ~1u);
                if (node == parent->child[0])
                    break;
                node = parent;
            } while (parent != sentinel);
            node = parent;
        }
    }
    // Phase 2: free keys/nodes postorder, then restore the empty sentinel invariant.
    FxTreeNode* root = sentinel->child[0];
    if (root) {
        FREE_RESOURCE_BRANCH(this, root->child[0]);
        FREE_RESOURCE_BRANCH(this, root->child[1]);
        if (KEY_IS_HEAP(root))
            appFreeUni__FPv(root->value);
        appFreeUni__FPv(root);
        spriteCacheCount = 0;
        spriteCacheRoot = 0;
        spriteCacheFirst = &spriteCacheRoot;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Delete cached spray templates in key order and free their owned string-key tree.
void CFxBox::ClearSprayTemplateCache() {
    FxTreeNode* node = (FxTreeNode*)sprayTemplateCacheFirst;
    FxTreeNode* sentinel = (FxTreeNode*)&sprayTemplateCacheRoot;
    FxTreeNode* root;
    // Phase 1: delete template objects, keeping their key nodes for traversal.
    while (node != sentinel) {
        SprayTemplateObject* object = (SprayTemplateObject*)node->cachedObject;
        delete object;
        FxTreeNext_Spray(node);
    }
    // Phase 2: release key/node storage and restore count/root/first together.
    root = (FxTreeNode*)sprayTemplateCacheRoot;
    if (root) {
        FxTreeNode* a = root->child[0];
        FREE_SPRAY_BRANCH(this, a);
        FxTreeNode* b = root->child[1];
        FREE_SPRAY_BRANCH(this, b);
        FreeSprayNodeValue(root);
        sprayTemplateCacheCount = 0;
        sprayTemplateCacheRoot = 0;
        sprayTemplateCacheFirst = &sprayTemplateCacheRoot;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Delete cached sequence templates in key order and free their owned string-key tree.
void CFxBox::ClearSequenceTemplateCache() {
    FxTreeNode* node = (FxTreeNode*)sequenceTemplateCacheFirst;
    FxTreeNode* sentinel = (FxTreeNode*)&sequenceTemplateCacheRoot;
    // Phase 1: delete sequence templates; iterator links remain valid until phase 2.
    while (node != sentinel) {
        SequenceTemplateObject* object = (SequenceTemplateObject*)node->cachedObject;
        delete object;
        FxTreeNext_Sequence(node);
    }
    // Phase 2: free key/node storage and restore the empty sentinel invariant.
    FxTreeNode* root = (FxTreeNode*)sequenceTemplateCacheRoot;
    if (root) {
        FxTreeNode* a = (root->child[0]);
        FREE_TEMPLATE_BRANCH(this, a);
        FxTreeNode* b = (root->child[1]);
        FREE_TEMPLATE_BRANCH(this, b);
        FreeTemplateNodeValue(root);
        sequenceTemplateCacheCount = 0;
        sequenceTemplateCacheRoot = 0;
        sequenceTemplateCacheFirst = &sequenceTemplateCacheRoot;
    }
}
#pragma pop
