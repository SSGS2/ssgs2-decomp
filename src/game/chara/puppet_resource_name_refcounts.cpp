#include <decomp/static_init.h>
#include "classes/MWStringStorage.h"

// Translation unit 0x8005E4D8-0x8005F290 (6 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct OwnedAllocation { void* vtable; void* allocation; };
struct NameRefCountTree { unsigned int count; struct CleanupNode* root; void* leftmost; };
// Partial owning-tree layout: count at +0, root link at +4, cached link at +8.
// Empty initialization stores null in root and points leftmost at the root slot.
// Nodes own their MWStringStorage key buffers; mapped values, if any, are not
// modeled here. Teardown frees keys and nodes, never repairs count/root/leftmost.
// A surviving wrapper therefore cannot be reused without reinitialization.
// Address-derived offsets retain the original initializer's integer store ordering.
enum { kNameTreeRoot = (int)&((NameRefCountTree*)0)->root,
       kNameTreeLeftmost = (int)&((NameRefCountTree*)0)->leftmost };

extern "C" {

extern float s_puppetResourceRuntimeFullTurn;
extern float kFloat_6_2831855;
extern unsigned char s_puppetResourceNameRefCounts[];
extern unsigned char s_puppetNameRefCountsDtorRegistration[];

void appFreeUni__FPv(void*);
void String_FreeHeapBuffer(void*, void*);
void __dl__FPv(void*);
void __register_global_object(void*, void (*)(void*), void*);
void* WPuppetResource_NameRefCountsDtor(void*, int);
void StringValueTree_DestroySubtree(void*, struct CleanupNode*);
void* dtor_8005E4D8(void*, int);

// Release only heap-backed string storage; inline characters belong to the object.
// object is a nullable borrowed MWStringStorage; unused is an ignored owner token.
// The string's fields are not cleared, so the same storage must not be freed twice.
void String_FreeHeapBuffer(void* unused, void* object) {
    // Both guards are retained: removing one changes CodeWarrior's inlining decisions.
    if (!object) {
        return;
    }
    if (!object) {
        return;
    }
    if ((AsMWStringStorage(object)->flags >> 31) != 0) {
        appFreeUni__FPv(AsMWStringStorage(object)->data);
    }
}

// Initialize the name-reference tree and register its static-lifetime destructor.
void WPuppetResource_InitNameRefCounts() {
    s_puppetResourceRuntimeFullTurn = kFloat_6_2831855;
    *reinterpret_cast<unsigned int*>(s_puppetResourceNameRefCounts) = 0;
    *reinterpret_cast<unsigned int*>(s_puppetResourceNameRefCounts + kNameTreeRoot) = 0;
    *reinterpret_cast<unsigned int*>(s_puppetResourceNameRefCounts + kNameTreeLeftmost) =
    reinterpret_cast<unsigned int>(s_puppetResourceNameRefCounts + kNameTreeRoot);
    __register_global_object(s_puppetResourceNameRefCounts,
    reinterpret_cast<void (*)(void*)>(WPuppetResource_NameRefCountsDtor),
    s_puppetNameRefCountsDtorRegistration);
}

struct CleanupNode {
    CleanupNode* left;
    CleanupNode* right;
    unsigned int taggedParent; // Unused link-header word; its bit encoding is not interpreted here.
    MWStringStorage key; // Inline or heap-owned name; release before freeing the node.
};

#pragma push
#pragma use_lmw_stmw on
// Destroy the string-keyed tree in postorder, then optionally delete its owner.
// Three levels are expanded before recursion to preserve the original call pattern.
// flags > 0 deletes object; the returned address may then be dangling. Null is safe.
void* dtor_8005E4D8(void* object, int flags) {
    if (object) {
        CleanupNode* root = ((NameRefCountTree*)object)->root;
        if (root) {
            // Phase 1: release the root's left subtree, visiting children before parents.
            CleanupNode* node = root->left;
            if (node) {
                CleanupNode* child = node->left;
                if (child) {
                    CleanupNode* descendant = child->left;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    CleanupNode* rightValue = child->right;
                    CleanupNode* rightValue2 = child->right;
                    descendant = rightValue2;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    String_FreeHeapBuffer(object, &child->key);
                    appFreeUni__FPv(child);
                }
                // Redundant reads/temporaries retain the matched inlining and lifetimes.
                __typeof__(node->right) unusedRightChildRead = node->right;
                CleanupNode* rightValue3 = node->right;
                child = rightValue3;
                if (child) {
                    CleanupNode* descendant = child->left;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    __typeof__(child->right) unusedRightDescendantRead = child->right;
                    CleanupNode* rightValue4 = child->right;
                    CleanupNode* rightValue5 = child->right;
                    descendant = rightValue5;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    String_FreeHeapBuffer(object, &child->key);
                    appFreeUni__FPv(child);
                }
                String_FreeHeapBuffer(object, &node->key);
                appFreeUni__FPv(node);
            }
            // Phase 2: the left subtree is gone; release the root's right subtree.
            CleanupNode* rightValue6 = root->right;
            node = rightValue6;
            if (node) {
                CleanupNode* leftValue = node->left;
                CleanupNode* child = leftValue;
                if (child) {
                    CleanupNode* descendant = child->left;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    CleanupNode* rightValue7 = child->right;
                    descendant = rightValue7;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    String_FreeHeapBuffer(object, &child->key);
                    appFreeUni__FPv(child);
                }
                child = node->right;
                if (child) {
                    CleanupNode* descendant = child->left;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    descendant = child->right;
                    if (descendant) {
                        if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                        if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                        String_FreeHeapBuffer(object, &descendant->key);
                        appFreeUni__FPv(descendant);
                    }
                    String_FreeHeapBuffer(object, &child->key);
                    appFreeUni__FPv(child);
                }
                String_FreeHeapBuffer(object, &node->key);
                appFreeUni__FPv(node);
            }
            // Phase 3: both child subtrees are gone; release the root key and node.
            String_FreeHeapBuffer(object, &root->key);
            appFreeUni__FPv(root);
        }
        // Node/key ownership is exhausted; a positive flag also releases the wrapper.
        if (flags > 0) __dl__FPv(object);
    }
    return object;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release the static resource-name tree and optionally delete its wrapper.
// Repeated null guards and redundant reads are retained to preserve matching.
// flags > 0 additionally deletes object; the return value is its original address.
void* WPuppetResource_NameRefCountsDtor(void* object, int flags) {
    if (object) {
        if (object) {
            if (object) {
                if (object) {
                    if (object) {
                        if (object) {
                            CleanupNode* root = ((NameRefCountTree*)object)->root;
                            if (root) {
                                // Phase 1: left subtree, including recursive cleanup below expanded levels.
                                CleanupNode* node = root->left;
                                if (node) {
                                    CleanupNode* child = node->left;
                                    if (child) {
                                        CleanupNode* descendant = child->left;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        CleanupNode* rightValue8 = child->right;
                                        descendant = rightValue8;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        String_FreeHeapBuffer(object, &child->key);
                                        appFreeUni__FPv(child);
                                    }
                                    CleanupNode* rightValue9 = node->right;
                                    CleanupNode* rightValue10 = node->right;
                                    CleanupNode* rightValue11 = node->right;
                                    child = rightValue10;
                                    if (child) {
                                        CleanupNode* descendant = child->left;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        CleanupNode* rightValue12 = child->right;
                                        CleanupNode* rightValue13 = child->right;
                                        descendant = rightValue12;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        String_FreeHeapBuffer(object, &child->key);
                                        appFreeUni__FPv(child);
                                    }
                                    String_FreeHeapBuffer(object, &node->key);
                                    appFreeUni__FPv(node);
                                }
                                // Phase 2: right subtree; do not dereference the already-freed left branch.
                                CleanupNode* rightValue14 = root->right;
                                node = rightValue14;
                                if (node) {
                                    CleanupNode* leftValue2 = node->left;
                                    CleanupNode* child = leftValue2;
                                    if (child) {
                                        CleanupNode* descendant = child->left;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        descendant = child->right;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        String_FreeHeapBuffer(object, &child->key);
                                        appFreeUni__FPv(child);
                                    }
                                    child = node->right;
                                    if (child) {
                                        CleanupNode* descendant = child->left;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        descendant = child->right;
                                        if (descendant) {
                                            if (descendant->left) StringValueTree_DestroySubtree(object, descendant->left);
                                            if (descendant->right) StringValueTree_DestroySubtree(object, descendant->right);
                                            String_FreeHeapBuffer(object, &descendant->key);
                                            appFreeUni__FPv(descendant);
                                        }
                                        String_FreeHeapBuffer(object, &child->key);
                                        appFreeUni__FPv(child);
                                    }
                                    String_FreeHeapBuffer(object, &node->key);
                                    appFreeUni__FPv(node);
                                }
                                // Phase 3: finish the root only after both branches have been released.
                                String_FreeHeapBuffer(object, &root->key);
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        // The wrapper is static in normal use; only a positive deleting flag frees it.
        if (0 < flags) __dl__FPv(object);
    }
    return object;
}
#pragma pop

// Free descendants before the root, releasing each node's owned name first.
// subtreeRoot must be a non-null detached subtree, without cycles or shared child nodes.
// owner is forwarded only as the ignored string-cleanup token; it is not freed.
void StringValueTree_DestroySubtree(void* owner, struct CleanupNode* subtreeRoot) {
    // Phase 1: destroy the left branch; deeper levels use this same postorder routine.
    CleanupNode* node = subtreeRoot->left;
    if (node) {
        CleanupNode* child = node->left;
        if (child) {
            CleanupNode* descendant = child->left;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            descendant = child->right;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            String_FreeHeapBuffer(owner, &child->key);
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (child) {
            CleanupNode* descendant = child->left;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            descendant = child->right;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            String_FreeHeapBuffer(owner, &child->key);
            appFreeUni__FPv(child);
        }
        String_FreeHeapBuffer(owner, &node->key);
        appFreeUni__FPv(node);
    }
    // Phase 2: destroy the right branch while the subtree root is still live.
    node = subtreeRoot->right;
    if (node) {
        CleanupNode* child = node->left;
        if (child) {
            CleanupNode* descendant = child->left;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            descendant = child->right;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            String_FreeHeapBuffer(owner, &child->key);
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (child) {
            CleanupNode* descendant = child->left;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            descendant = child->right;
            if (descendant) {
                if (descendant->left) {
                    StringValueTree_DestroySubtree(owner, descendant->left);
                }
                if (descendant->right) {
                    StringValueTree_DestroySubtree(owner, descendant->right);
                }
                String_FreeHeapBuffer(owner, &descendant->key);
                appFreeUni__FPv(descendant);
            }
            String_FreeHeapBuffer(owner, &child->key);
            appFreeUni__FPv(child);
        }
        String_FreeHeapBuffer(owner, &node->key);
        appFreeUni__FPv(node);
    }
    // Phase 3: release the root's key before freeing the root node allocation.
    String_FreeHeapBuffer(owner, &subtreeRoot->key);
    appFreeUni__FPv(subtreeRoot);
}

// Release an owned allocation and optionally delete its wrapper.
// flags > 0 also frees object; the returned address may be dangling. The member
// pointer is not reset, and no destructor is invoked on its untyped allocation.
void* OwnedAllocation_dtor(void* object, int flags) {
    if (object) {
        void* member = ((OwnedAllocation*)object)->allocation;
        if (member) {
            appFreeUni__FPv(member);
        }
        if (flags > 0) {
            __dl__FPv(object);
        }
    }
    return object;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FD8", _ctors_u8005E4D8_0, WPuppetResource_InitNameRefCounts);
#pragma pop
