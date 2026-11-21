#include <decomp/static_init.h>
#include <decomp/offset_of.h>
// Actor factory registry and string-map node storage (0x8029E9FC-0x8029F418).
// API/mod lookup:
// - Register an actor class: StringMap_SetByName(ActorFactoryRegistry_GetInstance(),
//   factory, name), e.g. IActor/CDummy/CBall registration in ball_actor_and_ambient_sound.cpp.
// - Resolve a class: TaskRegistry_CreateActorByName in task_registry_actor_and_debug_text.cpp searches this registry.
// - Lifetime: GetInstance owns process-lifetime two-tree storage; GMap_Dtor and
//   GMap_ClearOwnedEntries in string_map_tree_utils.cpp govern VALUE ownership (see header below).
// - Node/key storage only: GMapBase_DestroyStorage / StringTree_DestroyStorage.
//   These are lower-level teardown, not substitutes for GMap_Dtor or clear().
// Headers: StringTree8029EA / ResourceStringNode. No gameplay tuning knobs here;
// tau initialization is startup plumbing, not the actor selection policy.
#include "classes/StringTree8029EA.h"
#include "classes/ResourceStringNode.h"
#undef AsStringTree8029EA
// Interpret an existing registry header without changing its lifetime.
static inline StringTree8029EA* AsStringTree8029EA(void* tree) {
    return (StringTree8029EA*)tree;
}
// Interpret existing node storage without taking ownership.
static inline ResourceStringNode* AsResourceStringNode(void* node) {
    return (ResourceStringNode*)node;
}
// Translation unit 0x8029E9FC-0x8029F418 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern float kFloat_6_2831855;
extern float s_actorFactoryRegistryTau;
extern char s_actorFactoryInterfaceVtable[];
extern signed char s_actorFactoryRegistryInitialized;
extern char s_actorFactoryRegistry[];
extern char s_actorFactoryRegistryDtorRegistration[];
void __dl__FPv(void*);
void GMap_Dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void StringMap_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);

// Initialize the actor-factory unit's tau constant.
void ActorFactoryRegistry_InitializeTau() {
    s_actorFactoryRegistryTau = kFloat_6_2831855;
}

#pragma push
#pragma force_active on
STATIC_INIT_ENTRY_STATIC(".CRT$XCU804624A0", startup_fn_8029E9FC, ActorFactoryRegistry_InitializeTau);
#pragma pop

// Install the actor-factory interface vtable in an existing object.
void ActorFactoryInterface_InitVtable(void* self) {
    *(void**)self = &s_actorFactoryInterfaceVtable;
}

// Free the empty interface object only for a deleting destructor call.
void* dtor_8029EA18(void* self, int deleting) {
    if (self != 0 && deleting > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Both trees have the same three-word header. Ownership differs: GMap_Dtor in
// string_map_tree_utils.cpp calls GMap_ClearOwnedEntries, which DELETES first-tree values before
// freeing keys/nodes in both trees. Second-tree values are not deleted there.
// The storage-only destructors below never delete values in either tree; callers
// must finish value cleanup before using them for an owning tree.
struct RegistryTreeHeader { unsigned int headerWord; void* root; void* first; };
struct ActorFactoryRegistryStorage { RegistryTreeHeader trees[2]; };
typedef char RegistryTreeHeader_size[(sizeof(RegistryTreeHeader) == 12) ? 1 : -1];

} // extern "C"
enum RegistryHeaderOffsets {
    kRegistryFirstHeaderWord = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[0].headerWord),
    kRegistryFirstRoot = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[0].root),
    kRegistryFirstBegin = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[0].first),
    kRegistrySecondHeaderWord = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[1].headerWord),
    kRegistrySecondRoot = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[1].root),
    kRegistrySecondBegin = (int)DECOMP_OFFSETOF(ActorFactoryRegistryStorage, trees[1].first)
};
// Borrow one header word using a typed-layout offset; preserve Wii word store types.
template <int Offset> static inline unsigned int& RegistryHeaderWord() {
    return *(unsigned int*)(s_actorFactoryRegistry + Offset);
}

extern "C" {

// Lazily initialize the two empty registry trees and register their global destructor.
// Returns borrowed process-lifetime storage; callers must not delete it.
// Each 12-byte header is {word0, root, first}; first points to its own root slot
// when empty. This is unsynchronized lazy initialization, not a locking primitive.
void* ActorFactoryRegistry_GetInstance() {
    if (!s_actorFactoryRegistryInitialized) {
        RegistryHeaderWord<kRegistryFirstHeaderWord>() = 0;
        RegistryHeaderWord<kRegistryFirstRoot>() = 0;
        RegistryHeaderWord<kRegistryFirstBegin>() = (unsigned int)&RegistryHeaderWord<kRegistryFirstRoot>();
        RegistryHeaderWord<kRegistrySecondHeaderWord>() = 0;
        RegistryHeaderWord<kRegistrySecondRoot>() = 0;
        RegistryHeaderWord<kRegistrySecondBegin>() = (unsigned int)&RegistryHeaderWord<kRegistrySecondRoot>();
        __register_global_object(s_actorFactoryRegistry, GMap_Dtor, s_actorFactoryRegistryDtorRegistration);
        s_actorFactoryRegistryInitialized = 1;
    }
    return (void*)s_actorFactoryRegistry;
}

// Nodes have left/right/tagged-parent links at +0/+4/+8, a 12-byte small
// string key at +0xC, and an opaque value at +0x18. Bit 31 of stringFlags
// selects the heap pointer at +0x14; inline characters belong to the node.
// The cleanup routines free keys and nodes, not the opaque value pointers.
// The inline traversal below keeps the original left/right/root release order.

// Release a node's owned heap key, if any, then its allocation (not its value).
static inline void FreeRegistryStringNode(void* node) {
    // The key-subobject guard uses wrapping address arithmetic in the existing
    // binary. Keep it here, separate from traversal's real node-null checks.
    if (((unsigned int)(char*)node + 12) != 0 && ((unsigned int)(char*)node + 12) != 0) {
        if (AsResourceStringNode(node)->stringFlags >> 31)
            appFreeUni__FPv((void*)AsResourceStringNode(node)->heapData);
    }
    appFreeUni__FPv(node);
}

// Release a branch's children before its own key/node; do not touch its value.
static inline void DestroyRegistryBranch(void* self, void* node) {
    void* leftSubtree = AsResourceStringNode(node)->left;
    if (leftSubtree != 0) StringMap_DestroySubtree(self, leftSubtree);
    void* rightSubtree = AsResourceStringNode(node)->right;
    if (rightSubtree != 0) StringMap_DestroySubtree(self, rightSubtree);
    FreeRegistryStringNode(node);
}

#pragma use_lmw_stmw on
// Destroy string-key nodes in postorder, releasing heap strings, then optionally free the map.
// Precondition: any owned values have already been destroyed by the higher layer.
// GMap_Dtor must run that phase BEFORE this operation; calling this directly on
// a populated owning map leaks values. self may be null; no callbacks run here
// except allocation release. Do not reuse any node/iterator after this call.
// deleting > 0 also deletes self; the return value can be a freed address.
// Header links are not reset: this destructor is not a reusable clear operation.
void* GMapBase_DestroyStorage(void* self, int deleting) {
    if (self != 0) {
      // Preserve repeated inlined container guards required by this binary.
      if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
        void* root = AsStringTree8029EA(self)->root;
        if (root != 0) {
            // Phase 1: dispose the complete left branch in postorder. At the
            // deepest unrolled level, StringMap_DestroySubtree owns recursion.
            void* leftChild = AsResourceStringNode(root)->left;
            if (leftChild != 0) {
                void* leftGrandchild = AsResourceStringNode(leftChild)->left;
                if (leftGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(leftGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(leftGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(leftGrandchild);
                }
                void* rightGrandchild = AsResourceStringNode(leftChild)->right;
                if (rightGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(rightGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(rightGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(rightGrandchild);
                }
                FreeRegistryStringNode(leftChild);
            }
            // Phase 2: dispose the right branch after all left nodes are gone.
            void* rightChild = AsResourceStringNode(root)->right;
            if (rightChild != 0) {
                void* leftGrandchild = AsResourceStringNode(rightChild)->left;
                if (leftGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(leftGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(leftGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(leftGrandchild);
                }
                void* rightGrandchild = AsResourceStringNode(rightChild)->right;
                if (rightGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(rightGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(rightGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(rightGrandchild);
                }
                FreeRegistryStringNode(rightChild);
            }
            // Phase 3: both branches are gone; release the root key and node.
            FreeRegistryStringNode(root);
        }
      }
        // Phase 4: optionally release the containing allocation; return its address.
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy string-key nodes in postorder, then optionally free the tree object.
// Same ownership and deleting-flag contract as GMapBase_DestroyStorage above.
void* StringTree_DestroyStorage(void* self, int deleting) {
    if (self != 0) {
      // Preserve repeated inlined container guards required by this binary.
      if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
        void* root = AsStringTree8029EA(self)->root;
        if (root != 0) {
            // Phase 1: dispose the complete left branch in postorder. At the
            // deepest unrolled level, StringMap_DestroySubtree owns recursion.
            void* leftChild = AsResourceStringNode(root)->left;
            if (leftChild != 0) {
                void* leftGrandchild = AsResourceStringNode(leftChild)->left;
                if (leftGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(leftGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(leftGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(leftGrandchild);
                }
                void* rightGrandchild = AsResourceStringNode(leftChild)->right;
                if (rightGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(rightGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(rightGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(rightGrandchild);
                }
                FreeRegistryStringNode(leftChild);
            }
            // Phase 2: dispose the right branch after all left nodes are gone.
            void* rightChild = AsResourceStringNode(root)->right;
            if (rightChild != 0) {
                void* leftGrandchild = AsResourceStringNode(rightChild)->left;
                if (leftGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(leftGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(leftGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(leftGrandchild);
                }
                void* rightGrandchild = AsResourceStringNode(rightChild)->right;
                if (rightGrandchild != 0) {
                    void* leftGreatGrandchild = AsResourceStringNode(rightGrandchild)->left;
                    if (leftGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, leftGreatGrandchild);
                    }
                    void* rightGreatGrandchild = AsResourceStringNode(rightGrandchild)->right;
                    if (rightGreatGrandchild != 0) {
                        DestroyRegistryBranch(self, rightGreatGrandchild);
                    }
                    FreeRegistryStringNode(rightGrandchild);
                }
                FreeRegistryStringNode(rightChild);
            }
            // Phase 3: both branches are gone; release the root key and node.
            FreeRegistryStringNode(root);
        }
      }
        // Phase 4: optionally release the containing allocation; return its address.
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

} // extern "C"

