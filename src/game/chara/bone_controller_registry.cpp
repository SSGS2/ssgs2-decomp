// Translation unit 0x8025E8F8-0x8025F0BC (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Registry base: slots 0 and 1 are not used in this unit (purpose unknown); slot 2 releases the owned values.
class RegistryVirtual {
public:
    virtual void v0();
    virtual void v1();
    virtual void clearValues();
};
// Dispatch the registry cleanup hook through its original virtual slot.
void Registry_InvokeClearValuesHook(void* self) {
    ((RegistryVirtual*)self)->clearValues();
}

struct RegistryNode {
    RegistryNode* left;
    RegistryNode* right;
    unsigned int parentAndColor;
    unsigned int key;
    void* payload;
};

// Registry header: root doubles as the end-iterator address; first is the minimum node.
struct RegistryTreeView {
    void* vtable;
    void* owner;
    unsigned int nodeCount;
    RegistryNode* root;
    RegistryNode* first;
};
enum { kTreeColorBit = 1, kDefaultVectorComponentCount = 3 };

// Search the ordered tree for an exact key; return zero when lower_bound is the sentinel.
int BoneControllerTree_ContainsKey(void* tree, unsigned int* key) {
    RegistryNode* node = ((RegistryTreeView*)tree)->root;
    RegistryNode* last = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    while (node != 0) {
        if (node->key >= *key) {
            last = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (last != (RegistryNode*)&((RegistryTreeView*)tree)->root) {
        last = (*key >= last->key) ? last : (RegistryNode*)&((RegistryTreeView*)tree)->root;
    }
    if ((RegistryNode*)&((RegistryTreeView*)tree)->root == last) return 0;
    return 1;
}

// Delete each owned payload in key order while retaining the tree nodes.
int CRotateRegistry_DeleteAllValues(void* tree) {
    void* zero = 0;
    RegistryNode* node = ((RegistryTreeView*)tree)->first;
    RegistryNode* nil = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    while (nil != node) {
        void* payload = node->payload;
        if (payload != 0) {
            extern void __dl__FPv(void*);
            __dl__FPv(payload);
            node->payload = zero;
        }
        RegistryNode* child = node->right;
        if (child != 0) {
            while (child->left != 0)
                child = child->left;
            node = child;
        } else {
            RegistryNode* parent;
            do {
                parent = (RegistryNode*)(node->parentAndColor & ~((unsigned int)kTreeColorBit));
                if (node == parent->left) break;
                node = parent;
            } while (node != nil);
            node = parent;
        }
    }
    return 1;
}

extern void __dl__FPv(void*);
extern void RBTree_RebalanceAfterErase(void*, void*);
extern void appFreeUni__FPv(void*);

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Delete the mapped value, update the minimum iterator, rebalance and free its tree node.
int BoneControllerMap_EraseByIndex(void* tree, unsigned int* key) {
    RegistryNode* position = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    RegistryNode* current = ((RegistryTreeView*)tree)->root;
    while (current != 0) {
        if (current->key >= *key) {
            position = current;
            current = current->left;
        } else {
            current = current->right;
        }
    }
    if (position == (RegistryNode*)&((RegistryTreeView*)tree)->root || *key < position->key)
        position = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    if ((RegistryNode*)&((RegistryTreeView*)tree)->root == position) return 0;
    void* payload = position->payload;
    if (payload != 0) {
        __dl__FPv(payload);
        position->payload = 0;
    }

    RegistryNode* scan = ((RegistryTreeView*)tree)->root;
    RegistryNode* found = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    while (scan != 0) {
        if (scan->key >= *key) {
            found = scan;
            scan = scan->left;
        } else scan = scan->right;
    }
    if (found == (RegistryNode*)&((RegistryTreeView*)tree)->root || *key < found->key)
        found = (RegistryNode*)&((RegistryTreeView*)tree)->root;
    if (found != (RegistryNode*)&((RegistryTreeView*)tree)->root) {
        RegistryNode* end = ((RegistryTreeView*)tree)->first;
        position = found;
        if (found == end) {
            RegistryNode* child = found->right;
            if (child != 0) {
                RegistryNode* next;
                while ((next = child->left) != 0) child = next;
                found = child;
            } else {
                RegistryNode* parent;
                while (found != (parent = (RegistryNode*)(found->parentAndColor & ~((unsigned int)kTreeColorBit)))->left) {
                    found = parent;
                }
                found = parent;
            }
            ((RegistryTreeView*)tree)->first = found;
        }
        RegistryNode* root = ((RegistryTreeView*)tree)->root;
        RBTree_RebalanceAfterErase(position, root);
        appFreeUni__FPv(position);
        --((RegistryTreeView*)tree)->nodeCount;
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

struct BoneVectorEntry {
    unsigned int xBits, yBits, zBits, componentCount;
    float value;
};
void* BoneControllerVectorTree_FindInsertionPosition(void*, unsigned int*, RegistryNode**, unsigned char*, unsigned char*);
struct BoneVectorMapView { unsigned int reservedHeaderWord; unsigned int size; unsigned int nodeCount; RegistryNode* root; };
extern void* __nw__FUl(unsigned int);
extern RegistryNode* BoneControllerMap_InsertNode(void*, RegistryNode*, unsigned char, unsigned char, unsigned int*);
extern unsigned int s_boneControllerNullMappedValue;
extern const float kBoneVectorEntryDefaultValue;

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Copy a vector entry into the key map, allocating its payload when absent.
int BoneControllerMap_SetVectorEntry(BoneVectorMapView* self, unsigned int* key, const BoneVectorEntry* value) {
    unsigned int sz = self->size;
    if (sz <= *key) return 0;
    BoneVectorEntry* entry;
    RegistryNode* node = self->root;
    RegistryNode* where = (RegistryNode*)&self->root;
    while (node != 0) {
        if (node->key >= *key) {
            where = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (where == (RegistryNode*)&self->root || *key < where->key) where = (RegistryNode*)&self->root;
    if ((RegistryNode*)&self->root == where) {
        BoneVectorEntry* allocated = (BoneVectorEntry*)__nw__FUl(sizeof(BoneVectorEntry));
        if (allocated != 0) {
            allocated->componentCount = kDefaultVectorComponentCount;
            allocated->value = kBoneVectorEntryDefaultValue;
        }
        entry = allocated;
    } else {
        entry = (BoneVectorEntry*)where->payload;
    }
    if (entry == 0) return 0;
    *entry = *value;
    unsigned char flagB, flagA;
    RegistryNode* result;
    RegistryNode* parent = (RegistryNode*)BoneControllerVectorTree_FindInsertionPosition((void*)&self->nodeCount, key, &result, &flagA, &flagB);
    struct Pair { unsigned int key; BoneVectorEntry* value; };
    Pair* target;
    if (result == 0 || (target = (Pair*)&result->key)->key < *key) {
        unsigned int kv[2];
        kv[0] = *key;
        kv[1] = s_boneControllerNullMappedValue;
        target = (Pair*)&BoneControllerMap_InsertNode((void*)&self->nodeCount, parent, flagA, flagB, kv)->key;
    }
    target->value = entry;
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Find the insertion parent and predecessor, returning the left/unique insertion flags.
void* BoneControllerVectorTree_FindInsertionPosition(void* tree, unsigned int* key, RegistryNode** result,
                  unsigned char* flagA, unsigned char* flagB) {
    unsigned int zero = 0;
    unsigned int one = 1;
    *result = (RegistryNode*)zero;
    RegistryNode** arr = (RegistryNode**)tree;
    RegistryNode* last = (RegistryNode*)&arr[1];
    RegistryNode* node = arr[1];
    *flagA = (unsigned char)one;
    *flagB = (unsigned char)one;
    while (node != 0) {
        unsigned int value = *key;
        last = node;
        if (value < node->key) {
            node = node->left;
            *flagA = (unsigned char)one;
        } else {
            *result = node;
            node = node->right;
            *flagA = (unsigned char)zero;
            *flagB = (unsigned char)zero;
        }
    }
    return last;
}

struct TextureRegistryView {
    void* vtable;
    void* arg;
    unsigned int nodeCount;
    void* root;
    void* first;
};

// Initialize an empty texture-coordinate registry linked to its owner.
void TextureCoordinateRegistry_Init(TextureRegistryView* self, void* arg) {
    extern char s_textureCoordinateRegistryVtable[];
    self->vtable = s_textureCoordinateRegistryVtable;
    self->arg = arg;
    self->nodeCount = 0;
    self->root = 0;
    self->first = &self->root;
}

void RBTree_DestroySubtree(void*, void*);
#pragma use_lmw_stmw on
// Run the virtual cleanup hook, free the tree in postorder, and optionally free the registry.
// Shape of the tree teardown below (unrolled by the original compiler, four levels deep):
//   - nodes named by their path from the root (a = left, b = right, so "aba" = root->left->right->left);
//   - the deepest named nodes (aaa .. bbb) free their children with RBTree_DestroySubtree, then themselves;
//   - every node above them is freed after its children; the root is freed last.
// In plain C: free every node of the tree in postorder.
// The repeated `(void*)&self->nodeCount != 0` tests are always true (address of a member); they only exist to match.
void* TextureCoordinateRegistry_Dtor(TextureRegistryView* self, int freeSelf) {
    RegistryNode *root, *a, *aa, *aaa, *aab, *ab, *aba, *abb, *b, *ba, *baa, *bab, *bb, *bba, *bbb;
    if (self != 0) {
        extern char s_textureCoordinateRegistryVtable[];
        self->vtable = s_textureCoordinateRegistryVtable;
        ((RegistryVirtual*)self)->clearValues();
        if ((void*)&self->nodeCount != 0 && (void*)&self->nodeCount != 0 && (void*)&self->nodeCount != 0 && (void*)&self->nodeCount != 0 && (void*)&self->nodeCount != 0 && (void*)&self->nodeCount != 0) {
            root = (RegistryNode*)self->root;
            if (root != 0) {
                a = root->left;
                if (a != 0) {
                    aa = a->left;
                    if (aa != 0) {
                        aaa = aa->left;
                        if (aaa != 0) {
                            if (aaa->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aaa->left);
                            if (aaa->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aaa->right);
                            appFreeUni__FPv(aaa);
                        }
                        aab = aa->right;
                        if (aab != 0) {
                            if (aab->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aab->left);
                            if (aab->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aab->right);
                            appFreeUni__FPv(aab);
                        }
                        appFreeUni__FPv(aa);
                    }
                    ab = a->right;
                    if (ab != 0) {
                        aba = ab->left;
                        if (aba != 0) {
                            if (aba->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aba->left);
                            if (aba->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)aba->right);
                            appFreeUni__FPv(aba);
                        }
                        abb = ab->right;
                        if (abb != 0) {
                            if (abb->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)abb->left);
                            if (abb->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)abb->right);
                            appFreeUni__FPv(abb);
                        }
                        appFreeUni__FPv(ab);
                    }
                    appFreeUni__FPv(a);
                }
                b = root->right;
                if (b != 0) {
                    ba = b->left;
                    if (ba != 0) {
                        baa = ba->left;
                        if (baa != 0) {
                            if (baa->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)baa->left);
                            if (baa->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)baa->right);
                            appFreeUni__FPv(baa);
                        }
                        bab = ba->right;
                        if (bab != 0) {
                            if (bab->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bab->left);
                            if (bab->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bab->right);
                            appFreeUni__FPv(bab);
                        }
                        appFreeUni__FPv(ba);
                    }
                    bb = b->right;
                    if (bb != 0) {
                        bba = bb->left;
                        if (bba != 0) {
                            if (bba->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bba->left);
                            if (bba->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bba->right);
                            appFreeUni__FPv(bba);
                        }
                        bbb = bb->right;
                        if (bbb != 0) {
                            if (bbb->left) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bbb->left);
                            if (bbb->right) RBTree_DestroySubtree((void*)&self->nodeCount, (void*)bbb->right);
                            appFreeUni__FPv(bbb);
                        }
                        appFreeUni__FPv(bb);
                    }
                    appFreeUni__FPv(b);
                }
                appFreeUni__FPv(root);
            }
        }
        if (freeSelf > 0) __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

} // extern "C"
