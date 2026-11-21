#include "classes/CFxBox.h"
#define AsCFxBox(p) ((CFxBox*)(p))
#define FXBOX_FIELD_OFFSET(field) ((int)&((CFxBox*)0)->field)
#define FXNODE_FIELD_OFFSET(field) ((int)&((FxTreeNode*)0)->field)
// True when the node's string key is heap-allocated (bit 31 of `owns`) and must be freed with the node.
// The `&owns` operand is always non-null; it stays because removing it changes the generated code.
#define KEY_IS_HEAP(n) ((char*)(n) + FXNODE_FIELD_OFFSET(owns) && (((unsigned int)(n)->owns) >> 31))

// Translation unit 0x80108870-0x80109E1C (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

typedef CFxBox FxBoxState;

void CFxBox_ClearEffects(void*, int);
void CFxBox_ClearCachedResourceMap(void*);
void CFxBox_ClearSprayTemplateCache(void*);
void CFxBox_ClearSequenceTemplateCache(void*);
void CFxBox_ResetResources(void*);
void FxBoxResourceMap_dtor(void*, int);
void FxSequenceTemplateMap_dtor(void*, int);
void FxSprayTemplateMap_dtor(void*, int);
void FxSequenceTree_FreeSubtree(void*, void*);
void appFreeUni__FPv(void*);
void StringValueTree_DestroySubtree(void*, void*);
void __dl__FPv(void*);
extern char s_fxBoxVtable[];

// Tree contract: child[0]/[1] are left/right; parent bit 0 is colour, not address.
// Cache nodes carry a string key (owns bit 31 => value must be freed), then cachedObject/object.
// Each CFxBox cache stores count, root and first; the address of root is the end sentinel.
// Empty cache: count=0, root=0, first=&root. Payload destruction precedes node/key destruction.
struct FxTreeNode {
    FxTreeNode* child[2];
    FxTreeNode* parent;
    unsigned int owns;    char stringLength[4];
    void* value;
    void* cachedObject;
    void* object;
};

struct FxTreeOwner {
    void* nodeCount;
    FxTreeNode* root;
};

// Unrolling map: BRANCH -> PAIR -> LEAF -> external recursive destructor.
// Registry leaves free nodes only; cache leaves also free owned string keys.
// Outer unrolling levels stay macros to retain CodeWarrior register allocation.
// Their owner is explicit; node arguments must be stable lvalues with no side effects.
// Free both deeper registry subtrees and the non-null node; does not destroy a cache payload/key.
static inline void FreeSequenceLeaf(void* self, FxTreeNode*& n) {
    if ((n)->child[0]) FxSequenceTree_FreeSubtree(self, (n)->child[0]);
    if ((n)->child[1]) FxSequenceTree_FreeSubtree(self, (n)->child[1]);
    appFreeUni__FPv(n);
}

#define FREE_SEQUENCE_PAIR(owner, n) do { \
    if (n) { \
        FxTreeNode* pairLeft = (n)->child[0]; \
        if (pairLeft) FreeSequenceLeaf(owner, pairLeft); \
        FxTreeNode* pairRight = (n)->child[1]; \
        if (pairRight) FreeSequenceLeaf(owner, pairRight); \
        appFreeUni__FPv(n); \
    } \
} while (0)

#define FREE_SEQUENCE_BRANCH(owner, n) do { \
    if (n) { \
        FxTreeNode* treeLeft = (n)->child[0]; \
        FREE_SEQUENCE_PAIR(owner, treeLeft); \
        FxTreeNode* treeRight = (n)->child[1]; \
        FREE_SEQUENCE_PAIR(owner, treeRight); \
        appFreeUni__FPv(n); \
    } \
} while (0)

void FxBoxResourceMap_DestroySubtree(void*, void*);
void Object_DeleteVirtual(void*, void*);
void ResourceManager_ReleaseId(void*, void*);

// Free deeper sprite-cache subtrees, this node's owned key, then the node (null accepted).
static inline void FreeResourceLeaf(char* self, FxTreeNode*& n) {
    if (n) {
        void* eecItem = (void*)(n)->child[0];
        if (eecItem) FxBoxResourceMap_DestroySubtree(self + FXBOX_FIELD_OFFSET(spriteCacheCount), eecItem);
        eecItem = (void*)(n)->child[1];
        if (eecItem) FxBoxResourceMap_DestroySubtree(self + FXBOX_FIELD_OFFSET(spriteCacheCount), eecItem);
        if (KEY_IS_HEAP(n)) appFreeUni__FPv((n)->value);
        appFreeUni__FPv(n);
    }
}
#define FREE_RESOURCE_PAIR(owner, n) do { \
    if (n) { \
        FxTreeNode* eecLeft = (n)->child[0]; \
        FreeResourceLeaf(owner, eecLeft); \
        FxTreeNode* eecRight = (n)->child[1]; \
        FreeResourceLeaf(owner, eecRight); \
        if (KEY_IS_HEAP(n)) appFreeUni__FPv((n)->value); \
        appFreeUni__FPv(n); \
    } \
} while (0)
#define FREE_RESOURCE_BRANCH(owner, n) do { \
    if (n) { \
        /* own names: FREE_RESOURCE_PAIR declares eecLeft/eecRight itself, so passing eecRight made its */ \
        /* trailing frees hit the inner leaf (double free, pair node leaked). The original frees */ \
        /* each pair node once (0x801093C0 bl appFreeUni on the right pair, before the root node). */ \
        FxTreeNode* treeLeft = (n)->child[0]; \
        FREE_RESOURCE_PAIR(owner, treeLeft); \
        FxTreeNode* treeRight = (n)->child[1]; \
        FREE_RESOURCE_PAIR(owner, treeRight); \
        if (KEY_IS_HEAP(n)) appFreeUni__FPv((n)->value); \
        appFreeUni__FPv(n); \
    } \
} while (0)

#pragma push
#pragma use_lmw_stmw on
// Release cached objects and resource IDs in key order, then free the owned string-key nodes.
void CFxBox_ClearCachedResourceMap(void* ptr) {
    char* self = (char*)ptr;
    FxTreeNode* sentinel = (FxTreeNode*)(self + FXBOX_FIELD_OFFSET(spriteCacheRoot));
    FxTreeNode* node = *(FxTreeNode**)(self + FXBOX_FIELD_OFFSET(spriteCacheFirst));
    // Phase 1: release both payload handles while the tree links still exist.
    while (node != sentinel) {
        void* owner = *(void**)(self + FXBOX_FIELD_OFFSET(resourceManager));
        if (owner) {
            void* first = *(void**)((char*)node + FXNODE_FIELD_OFFSET(cachedObject));
            if (first) {
                Object_DeleteVirtual(owner, first);
                *(void**)((char*)node + FXNODE_FIELD_OFFSET(cachedObject)) = 0;
            }
            void* second = *(void**)((char*)node + FXNODE_FIELD_OFFSET(object));
            if (second) {
                ResourceManager_ReleaseId(owner, second);
                *(void**)((char*)node + FXNODE_FIELD_OFFSET(object)) = 0;
            }
        }
        // In-order successor: descend right/left, otherwise climb untagged parents.
        FxTreeNode* next = node->child[1];
        if (next) {
            node = next;
            while (node->child[0]) node = node->child[0];
        } else {
            FxTreeNode* parent;
            do {
                parent = (FxTreeNode*)((unsigned int)node->parent & ~1u);
                if (node == parent->child[0]) break;
                node = parent;
            } while (parent != sentinel);
            node = parent;
        }
    }
    // Phase 2: free keys/nodes postorder, then restore the empty sentinel invariant.
    FxTreeNode* root = sentinel->child[0];
    if (root) {
        FREE_RESOURCE_BRANCH(self, root->child[0]);
        FREE_RESOURCE_BRANCH(self, root->child[1]);
        /* original 0x801093C4-0x801093E8: the root node's own owned value is freed before the node */
        if (KEY_IS_HEAP(root)) appFreeUni__FPv(root->value);
        appFreeUni__FPv(root);
        *(void**)(self + FXBOX_FIELD_OFFSET(spriteCacheCount)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(spriteCacheRoot)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(spriteCacheFirst)) = self + FXBOX_FIELD_OFFSET(spriteCacheRoot);
    }
}
#pragma pop

// Free an owned spray-template string key and its non-null node; payload deletion precedes this call.
static inline void FreeSprayNodeValue(char* self, FxTreeNode*& n) {
    if ((char*)(n) + FXNODE_FIELD_OFFSET(owns)) {
        if (KEY_IS_HEAP(n)) appFreeUni__FPv((n)->value);
    }
    appFreeUni__FPv(n);
}
#define FREE_SPRAY_LEAF(owner, n) do { \
    if (n) { \
        void* item = (void*)(n)->child[0]; if (item) StringValueTree_DestroySubtree(owner + FXBOX_FIELD_OFFSET(sprayTemplateCacheCount), item); \
        item = (void*)(n)->child[1]; if (item) StringValueTree_DestroySubtree(owner + FXBOX_FIELD_OFFSET(sprayTemplateCacheCount), item); \
        FreeSprayNodeValue(owner, n); \
    } \
} while (0)
#define FREE_SPRAY_PAIR(owner, n) do { \
    if (n) { \
        FxTreeNode* pairLeft = (n)->child[0]; FREE_SPRAY_LEAF(owner, pairLeft); \
        FxTreeNode* pairRight = (n)->child[1]; FREE_SPRAY_LEAF(owner, pairRight); \
        FreeSprayNodeValue(owner, n); \
    } \
} while (0)
#define FREE_SPRAY_BRANCH(owner, n) do { \
    if (n) { \
        FxTreeNode* treeLeft = (n)->child[0]; FREE_SPRAY_PAIR(owner, treeLeft); \
        FxTreeNode* treeRight = (n)->child[1]; FREE_SPRAY_PAIR(owner, treeRight); \
        FreeSprayNodeValue(owner, n); \
    } \
} while (0)

struct SprayTemplateObject {
    virtual ~SprayTemplateObject();
};

// Advance to the in-order successor, masking the tree-colour bit from parent pointers.
// (Same routine as FxTreeNext_Sequence below; kept as two copies, one per function.)
inline void FxTreeNext_Spray(FxTreeNode*& node) {
    FxTreeNode* next = node->child[1];
    if (next) {
        while (next->child[0]) next = next->child[0];
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
// Delete cached spray templates in key order and free their owned string-key tree.
void CFxBox_ClearSprayTemplateCache(void* ptr) {
    char* self = (char*)ptr;
    FxTreeNode* node = *(FxTreeNode**)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheFirst));
    FxTreeNode* sentinel = (FxTreeNode*)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheRoot));
    FxTreeNode* root;
    // Phase 1: delete template objects, keeping their key nodes for traversal.
    while (node != sentinel) {
        SprayTemplateObject* object = *(SprayTemplateObject**)((char*)node + FXNODE_FIELD_OFFSET(cachedObject));
        delete object;
        FxTreeNext_Spray(node);
    }
    // Phase 2: release key/node storage and restore count/root/first together.
    root = *(FxTreeNode**)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheRoot));
    if (root) {
        FxTreeNode* a = root->child[0];
        FREE_SPRAY_BRANCH(self, a);
        FxTreeNode* b = root->child[1];
        FREE_SPRAY_BRANCH(self, b);
        FreeSprayNodeValue(self, root);
        *(void**)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheCount)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheRoot)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(sprayTemplateCacheFirst)) = self + FXBOX_FIELD_OFFSET(sprayTemplateCacheRoot);
    }
}
#pragma pop

// Free an owned sequence-template string key and its non-null node; children were already released.
static inline void FreeTemplateNodeValue(char* self, FxTreeNode*& n) {
    if ((char*)(n) + FXNODE_FIELD_OFFSET(owns)) {
        if ((char*)(n) + FXNODE_FIELD_OFFSET(owns) && ((n)->owns >> 31)) appFreeUni__FPv((n)->value);
    }
    appFreeUni__FPv(n);
}
#define FREE_TEMPLATE_LEAF(owner, n) do { \
    if (n) { \
        void* item = (n)->child[0]; if (item) StringValueTree_DestroySubtree(owner + FXBOX_FIELD_OFFSET(sequenceTemplateCacheCount), item); \
        item = (n)->child[1]; if (item) StringValueTree_DestroySubtree(owner + FXBOX_FIELD_OFFSET(sequenceTemplateCacheCount), item); \
        FreeTemplateNodeValue(owner, n); \
    } \
} while (0)
#define FREE_TEMPLATE_PAIR(owner, n) do { \
    if (n) { \
        FxTreeNode* pairLeft = (n)->child[0]; FREE_TEMPLATE_LEAF(owner, pairLeft); \
        FxTreeNode* pairRight = (n)->child[1]; FREE_TEMPLATE_LEAF(owner, pairRight); \
        FreeTemplateNodeValue(owner, n); \
    } \
} while (0)
#define FREE_TEMPLATE_BRANCH(owner, n) do { \
    if (n) { \
        FxTreeNode* treeLeft = (n)->child[0]; FREE_TEMPLATE_PAIR(owner, treeLeft); \
        FxTreeNode* treeRight = (n)->child[1]; FREE_TEMPLATE_PAIR(owner, treeRight); \
        FreeTemplateNodeValue(owner, n); \
    } \
} while (0)
#pragma push
#pragma use_lmw_stmw on
struct SequenceTemplateObject {
    virtual ~SequenceTemplateObject();
};
// Advance to the in-order successor, masking the tree-colour bit from parent pointers.
inline void FxTreeNext_Sequence(FxTreeNode*& node) {
    FxTreeNode* next = node->child[1];
    if (next) {
        while (next->child[0]) next = next->child[0];
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
// Delete cached sequence templates in key order and free their owned string-key tree.
void CFxBox_ClearSequenceTemplateCache(void* ptr) {
    char* self = (char*)ptr;
    FxTreeNode* node = *(FxTreeNode**)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheFirst));
    FxTreeNode* sentinel = (FxTreeNode*)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheRoot));
    // Phase 1: delete sequence templates; iterator links remain valid until phase 2.
    while (node != sentinel) {
        SequenceTemplateObject* object = *(SequenceTemplateObject**)((char*)node + FXNODE_FIELD_OFFSET(cachedObject));
        delete object;
        FxTreeNext_Sequence(node);
    }
    // Phase 2: free key/node storage and restore the empty sentinel invariant.
    FxTreeNode* root = *(FxTreeNode**)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheRoot));
    if (root) {
        FxTreeNode* a = (root->child[0]);
        FREE_TEMPLATE_BRANCH(self, a);
        FxTreeNode* b = (root->child[1]);
        FREE_TEMPLATE_BRANCH(self, b);
        FreeTemplateNodeValue(self, root);
        *(void**)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheCount)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheRoot)) = 0;
        *(void**)(self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheFirst)) = self + FXBOX_FIELD_OFFSET(sequenceTemplateCacheRoot);
    }
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release the spray-sequence registry nodes and optionally delete the registry storage.
void* FxSprayTree_dtor(FxTreeNode* self, int flags) {
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
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release the effect-sequence registry nodes and optionally delete the registry storage.
FxTreeNode* FxSequenceTree_dtor(FxTreeNode* self, int flags) {
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
      if (flags > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Reset resources, destroy each effect registry, and optionally delete the effect box.
FxBoxState* CFxBox_dtor(FxBoxState* self, int flags) {
    if (self) {
        AsCFxBox(self)->vtbl = s_fxBoxVtable; // CFxBox_dtor: +0x0
        CFxBox_ResetResources(self);
        FxSequenceTree_dtor((FxTreeNode*)((char*)&AsCFxBox(self)->sequenceCount), -1);
        FxSprayTree_dtor((FxTreeNode*)((char*)&AsCFxBox(self)->sprayCount), -1);
        FxBoxResourceMap_dtor((char*)(char*)&AsCFxBox(self)->spriteCacheCount, -1);
        FxSequenceTemplateMap_dtor((char*)(char*)&AsCFxBox(self)->sequenceTemplateCacheCount, -1);
        FxSprayTemplateMap_dtor((char*)(char*)&AsCFxBox(self)->sprayTemplateCacheCount, -1);
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Store a borrowed resource-manager address in the existing 32-bit integer ABI; no retain/release.
void CFxBox_SetResourceManager(FxBoxState* self, int resourceManagerAddress) {
    self->resourceManager = resourceManagerAddress;
}

// Store a borrowed camera-transform address in the existing 32-bit integer ABI; no copy is made.
void CFxBox_SetCameraTransform(FxBoxState* self, int cameraTransformAddress) {
    self->cameraTransformAddress = cameraTransformAddress;
}

// Clear live effects before releasing sprite, spray-template, and sequence-template caches.
void CFxBox_ResetResources(void* self) {
    CFxBox_ClearEffects(self, 1);
    CFxBox_ClearCachedResourceMap(self);
    CFxBox_ClearSprayTemplateCache(self);
    CFxBox_ClearSequenceTemplateCache(self);
}

} // extern "C"
