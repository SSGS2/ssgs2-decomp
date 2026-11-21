#include <decomp/cw_bitfields.h>
// Translation unit 0x8029F418-0x8029FAC4 (4 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

// Borrow the smaller operand (the second one on equality).
template <class T> static inline const T& Min(const T& a, const T& b) { return a < b ? a : b; }

extern "C" {

struct Str;
struct Node;
struct Tree;
struct Alloc;

void appFreeUni__FPv(void* p);
void String_ReserveCapacity(Str*, unsigned int);
Str* String_ReplaceRange(Str*, unsigned int, unsigned int, const char*, const char*, const Alloc&);
unsigned strlen(const char*);
int memcmp(const char*, const char*, unsigned);
void* __dl__FPv(void*);
void GMap_ClearOwnedEntries(void*);
void StringTree_DestroyStorage(void*, int);
void GMapBase_DestroyStorage(void*, int);
Node* StringMap_InsertNode(Tree*, Node*, bool, bool, const void*);

extern void* s_stringMapDefaultValue;

struct Alloc { char c __attribute__((aligned(4))); };

struct Str {
    union {
        unsigned int flag;
        struct { CW_BITS2( unsigned int heapbit:1, unsigned int capacity:31) };
        struct { CW_BITS2( unsigned char byteheap:1, unsigned char smallsize:7) };
    };
    unsigned int size;
    char* ptr;

    // Initialize the packed inline representation to an empty string.
    Str() { flag = 0; size = 0; ptr = 0; }
    // Copy a terminated name into this string's own storage.
    Str(const char* s) {
        flag = 0; size = 0; ptr = 0;
        unsigned int n = strlen(s);
        String_ReserveCapacity(this, n);
        Alloc a;
        Alloc copy = a;
        String_ReplaceRange(this, 0, 0, s, s + n, copy);
    }
    // Copy inline bytes directly or allocate an independent heap string.
    Str(const Str& s) {
        unsigned int f = s.flag;
        if (!(f >> 31)) {
            flag = f; size = s.size; ptr = s.ptr;
        } else {
            flag = 0; size = 0; ptr = 0;
            String_ReserveCapacity(this, s.size);
            Alloc a;
            Alloc copy = a;
            String_ReplaceRange(this, 0, 0, s.ptr, s.ptr + s.size, copy);
        }
    }


    // Decode logical byte length from the active inline/heap view.
    unsigned int length() const { return !heapbit ? smallsize : size; }
    // Borrow character storage; inline bytes start one byte past the tag.
    const char* data() const { return heapbit ? ptr : (const char*)this + 1; }
};

struct LocalStr : Str {
    // Own a temporary copy of the lookup key.
    LocalStr(const char* s) : Str(s) {}
    // Release the temporary's heap characters, if any.
    ~LocalStr() { if (flag >> 31) appFreeUni__FPv(ptr); }
};
struct Pair_u8029F418 {
    Str key;
    void* value;
    // Copy the owned key and borrow the supplied value pointer.
    Pair_u8029F418(const Str& k, void* const& v) : key(k), value(v) {}
    // Destroy only key storage; value ownership is handled by the map's first-tree pass.
    ~Pair_u8029F418() { if (key.flag >> 31) appFreeUni__FPv(key.ptr); }
};

struct Node {
    Node* left;
    Node* right;
    Node* parent;
    Pair_u8029F418 pair;
};

struct Tree {
    int count;
    Node header;
};

struct Map {
    char firstTreeStorage[0xC]; // the second tree follows this first tree header
    Tree tree;
};

// Decode a packed key into borrowed bytes and logical byte length.
static inline void GetStr(const Str* s, const char*& bytes, unsigned int& length) {
    if (!s->heapbit) { bytes = (const char*)s + 1; length = s->smallsize; }
    else { bytes = s->ptr; length = s->size; }
}

// Compare lengths alone (unused by the lexical-comparison chains below).
static inline int CompareLengths(unsigned int leftLength, unsigned int rightLength) {
    return leftLength < rightLength ? -1 : (leftLength == rightLength ? 0 : 1);
}
// Compare common-prefix bytes, then order equal prefixes by their full lengths.
// rightPrefixLength aliases rightLength through the call chain; both describe the right key length.
static inline int CompareRaw(const char* leftData, unsigned int leftLength, const char* rightData, const unsigned int& rightPrefixLength, unsigned int rightLength) {
    int comparison = memcmp(leftData, rightData, Min(rightPrefixLength, leftLength));
    if (comparison == 0) {
        if (leftLength < rightLength) comparison = -1;
        else if (leftLength == rightLength) comparison = 0;
        else comparison = 1;
    }
    return comparison;
}
// Bound the left comparison to min(requested prefix leftPrefixLimit, stored length leftStoredLength).
static inline int CompareSub(const char* leftData, unsigned int leftStoredLength, const unsigned int& leftPrefixLimit, const char* rightData, const unsigned int& rightPrefixLength, unsigned int rightLength) {
    return CompareRaw(leftData, Min(leftPrefixLimit, leftStoredLength), rightData, rightPrefixLength, rightLength);
}
// Decode the left key and compare a bounded prefix against right-hand bytes.
static inline int ComparePart(const Str* a, unsigned int leftPrefixLimit, const char* rightData, unsigned int rightLength) {
    const char* leftData;
    unsigned int leftStoredLength;
    GetStr(a, leftData, leftStoredLength);
    return CompareSub(leftData, leftStoredLength, leftPrefixLimit, rightData, rightLength, rightLength);
}
// Compare the WHOLE left key with bytes rightData/rightLength; this is not a length-only test.
static inline int CompareWholeKeyToBytes(const Str* a, const char* rightData, unsigned int rightLength) {
    return ComparePart(a, a->length(), rightData, rightLength);
}
// Compare string keys lexicographically, using length to break equal-prefix ties.
static int CompareKeys(const Str* a, const Str* b) {
    const char* rightData;
    unsigned int rightLength;
    GetStr(b, rightData, rightLength);
    return CompareWholeKeyToBytes(a, rightData, rightLength);
}
// Delete owned map values, release the string tree and base map, then optionally free storage.
void* GMap_Dtor(Map* self, int deleting) {
    if (self) {
        GMap_ClearOwnedEntries(self);
        StringTree_DestroyStorage(&self->tree, -1);
        GMapBase_DestroyStorage(self, -1);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Search uses the same lexical order as CompareKeys. The parallel helper chain
// preserves the original return paths under this distinct optimization pragma.
 #pragma opt_loop_invariants off
// Compare raw search-key bytes, breaking equal-prefix ties by length.
static inline int TargetRaw(const char* leftData, unsigned int leftLength, const char* rightData, const unsigned int& rightPrefixLength, unsigned int rightLength) {
    int comparison = memcmp(leftData, rightData, Min(rightPrefixLength, leftLength));
    if (comparison == 0) {
        if (leftLength < rightLength) comparison = -1;
        else if (leftLength == rightLength) return 0;
        else return 1;
    }
    return comparison;
}
// Clamp the search-key prefix before its byte comparison.
static inline int TargetSub(const char* leftData, unsigned int leftStoredLength, const unsigned int& leftPrefixLimit, const char* rightData, const unsigned int& rightPrefixLength, unsigned int rightLength) {
    return TargetRaw(leftData, Min(leftPrefixLimit, leftStoredLength), rightData, rightPrefixLength, rightLength);
}
// Decode the search key before comparing its requested prefix.
static inline int TargetPart(const Str* a, unsigned int leftPrefixLimit, const char* rightData, unsigned int rightLength) {
    const char* leftData;
    unsigned int leftStoredLength;
    GetStr(a, leftData, leftStoredLength);
    return TargetSub(leftData, leftStoredLength, leftPrefixLimit, rightData, rightLength, rightLength);
}
// Compare the whole search key to rightData/rightLength using bytes and then length.
static inline int CompareSearchKeyToBytes(const Str* a, const char* rightData, unsigned int rightLength) {
    return TargetPart(a, a->length(), rightData, rightLength);
}
// Compare the search key with a stored node key in the tree's lexical order.
static inline int TargetKeys(const Str* a, const Str* b) {
    const char* rightData;
    unsigned int rightLength;
    GetStr(b, rightData, rightLength);
    return CompareSearchKeyToBytes(a, rightData, rightLength);
}
// Find the insertion parent and nearest key candidate in the string-keyed tree.
Node* StringMap_FindInsertionPosition(Tree* tree, const Str* key, Node** found, bool* insertLeft, bool* beforeFirstCandidate) {
    Node* last = (Node*)&tree->header;
    *found = 0;
    Node* node = tree->header.left;
    *insertLeft = 1;
    *beforeFirstCandidate = 1;
    while (node != 0) {
        last = node;
        if (TargetKeys(key, &node->pair.key) < 0) {
            node = node->left;
            *insertLeft = 1;
        } else {
            *found = node;
            node = node->right;
            *insertLeft = 0;
            *beforeFirstCandidate = 0;
        }
    }
    return last;
}

#pragma opt_loop_invariants on
// Insert a missing string key, then store the supplied value under that name.
// Copies name into node-owned storage; value itself is not copied. Existing
// values are overwritten without deletion, so the caller must arrange release
// if needed. ClearOwnedEntries deletes first-tree values; this setter targets
// the second tree, whose cleanup frees only keys/nodes. Its value ownership must
// be managed elsewhere; the visible setter does not establish an alias relation.
void StringMap_SetByName(Map* self, void* value, const char* name) {
    struct { bool insertLeft; bool beforeFirstCandidate; } flags;
    Node* found;
    LocalStr key(name);
    Node* last = StringMap_FindInsertionPosition(&self->tree, &key, &found, &flags.insertLeft, &flags.beforeFirstCandidate);
    Pair_u8029F418* payload;
    // Insertion copies the temporary pair before it leaves scope. After either
    // branch, payload points into the tree, never at the expired temporary.
    if (found == 0 || (payload = &found->pair, CompareKeys(&payload->key, &key) < 0)) {
        Pair_u8029F418 p(key, s_stringMapDefaultValue);
        payload = &p;
        payload = &StringMap_InsertNode(&self->tree, last, flags.insertLeft, flags.beforeFirstCandidate, payload)->pair;
    }
    payload->value = value;
}

struct OwnedMapObject;

struct GNode {
    GNode* left;
    GNode* right;
    unsigned int parent;
    Pair_u8029F418 pair;
};

struct GTree {
    int count;
    GNode* root;
    GNode* first;
};

struct OwnedMapTrees {
    GTree t1;
    GTree t2;
};
// Two compact 12-byte headers: [count, root, first]. The synthetic end node is
// the ADDRESS of each header's root slot (+4), not the root pointer value and
// not an allocated node. Thus &t2.count-8 and &t1.count+1 denote the same end.

struct OwnedMapObject {
    virtual void v0();
    virtual ~OwnedMapObject();
};

void RBTree_RebalanceAfterErase(void*, void*);

// Advance a live, non-end iterator; mask the parent link's low color bit.
// Take the leftmost right descendant, or climb until arriving from a left child.
// Kept as a macro because inline forms change this unit's register allocation.
// Argument must be a side-effect-free pointer lvalue. Expansion declares r_ in
// the caller scope and therefore must appear in its own enclosing block.
#define ADVANCE_OWNED_MAP_NODE(n) GNode* r_ = (n)->right; \
    if (r_ != 0) { \
        while (r_->left != 0) r_ = r_->left; \
        (n) = r_; \
    } else { \
        GNode* p_; \
        while (p_ = (GNode*)((n)->parent & ~1u), (n) != p_->left) (n) = p_; \
        (n) = p_; \
    }

// Free only a node's heap key, leaving its value and node allocation untouched.
static inline void destroyNodeKey(Pair_u8029F418* p) {
    if (p != 0) {
        if (p != 0 && p->key.flag >> 31) {
            appFreeUni__FPv(p->key.ptr);
        }
    }
}

#pragma opt_lifetimes off
#pragma function_align 4
// Delete values owned by the first tree, then erase and free nodes and keys in both trees.
void GMap_ClearOwnedEntries(void* mapStorage) {
#define S ((OwnedMapTrees*)mapStorage)
    GNode* firstTreeEnd = (GNode*)(&S->t1.count + 1);
    GNode* node = S->t1.first;

    while (node != firstTreeEnd) {
        delete (OwnedMapObject*)node->pair.value;
        ADVANCE_OWNED_MAP_NODE(node)
    }

    GNode* n = S->t1.first;
    // Values are gone; now erase owning-tree nodes, releasing their copied keys.
    // t1's sentinel (its header node) computed from t2's count field: a two-step address keeps it a
    // separate value, so the compiler does not fold the root load below into 0x0(reg).
    GNode* e = (GNode*)((char*)&S->t2.count - 8);
    while (n != e) {
        node = n;
        ADVANCE_OWNED_MAP_NODE(n)
        GNode* t = node;
        if (node == S->t1.first) {
            ADVANCE_OWNED_MAP_NODE(t)
            S->t1.first = t;
        }
        RBTree_RebalanceAfterErase(node, S->t1.root);
        destroyNodeKey(&node->pair);
        appFreeUni__FPv(node);
        S->t1.count = S->t1.count - 1;
    }

    // Second tree: erase keys/nodes only; this pass does not delete its values.
    GNode* secondTreeEnd = (GNode*)((char*)&S->t2 + 4);
    GNode* secondTreeNode = S->t2.first;
    while (secondTreeNode != secondTreeEnd) {
        node = secondTreeNode;
        ADVANCE_OWNED_MAP_NODE(secondTreeNode)
        GNode* t = node;
        if (node == S->t2.first) {
            ADVANCE_OWNED_MAP_NODE(t)
            S->t2.first = t;
        }
        RBTree_RebalanceAfterErase(node, S->t2.root);
        destroyNodeKey(&node->pair);
        appFreeUni__FPv(node);
        S->t2.count = S->t2.count - 1;
    }

#undef S
}


#pragma opt_lifetimes on
} // extern "C"

