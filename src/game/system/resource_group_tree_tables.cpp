#pragma ipa file
#pragma auto_inline off
// Translation unit 0x800E4CD4-0x800E5FFC (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/ResourceLookupTree.h"
#include "classes/ResourceGroupNode.h"
#include "classes/ResourceStringNode.h"

// Container guide: the owner's third table is at +0x2E8. It maps byte group keys
// to nested string-key maps. Each three-word tree header is count/root/first;
// &root is the end sentinel, not a heap node. Nodes have left/right/tagged-parent
// links at +0/+4/+8 and payload at +0xC; parent bit 0 stores color.
// A group payload is key + padding + a nested header. A string payload is a
// 12-byte inline/heap string followed by a resource pointer. Heap string bit 31
// owns its text. Tree cleanup frees strings/nodes; resource release is a separate
// step in ResourceGroups_ClearThirdTable. Lookups return borrowed values.
// Third of the resource owner's group tables, retaining its byte-sized lookup key.
struct ResourceThirdTableView {
    char reserved[0x2E8];
    ResourceLookupTree groups;
};
struct AnimationValueView { unsigned char key; char alignment[3]; ResourceLookupTree animations; };
struct NamedResourceValueView { char stringStorage[12]; void* resource; };
// The owner's third group table (+0x2E8): byte group key -> nested string-key map.
static inline ResourceLookupTree& ThirdTable(void* owner) { return ((ResourceThirdTableView*)owner)->groups; }
#pragma push
#pragma always_inline on
// Borrow the outer byte-group node; callers must exclude the end sentinel.
static inline ResourceGroupNode* GroupNode(void* p) { return (ResourceGroupNode*)p; }
// Borrow the inner string-key node; resource pointers remain owned by the resource table.
static inline ResourceStringNode* StringNode(void* p) { return (ResourceStringNode*)p; }
#pragma pop

enum { kU32SignBit = 31, kTaggedParentColorBit = 1, kStringHeapFlagBit = 31, kInlineStringLengthMask = 0x7F };

extern "C" {

typedef unsigned int u32;

struct ByteKeyTreeNode {
    ByteKeyTreeNode* left;
    ByteKeyTreeNode* right;
    void* parent;
    unsigned char key;
};

// Compare iterator words without assuming signed ordering.
unsigned int CameraWorkTableIterator_NotEqualWords(const u32* a, const u32* b) {
    u32 y;
    u32 x;
    x = *a;
    y = *b;
    return ((y - x) | (x - y)) >> kU32SignBit;
}

// Requires a dereferenceable iterator (not end). anchor receives its old node;
// node receives its successor. Neither pointer acquires ownership.
// Post-increment the outer byte-key tree iterator, masking the parent color tag.
void CameraWorkTableIterator_PostIncrement(void** anchor, void** node) {
    *anchor = *node;
    ByteKeyTreeNode* cur = (ByteKeyTreeNode*)*node;
    cur = cur->right;
    if (cur != 0) {
        while (cur->left != 0) cur = cur->left;
        *node = cur;
    } else {
        for (;;) {
            cur = (ByteKeyTreeNode*)*node;
            ByteKeyTreeNode* parent = (ByteKeyTreeNode*)((u32)cur->parent & ~(u32)kTaggedParentColorBit);
            *node = parent;
            if (cur == parent->left) break;
        }
    }
}

// predecessorOut is the greatest encountered key <= *key, or null. leftOut
// selects the parent's link; noPredecessorOut stays 1 until a right descent.
// No node is inserted here, and all output pointers must be writable.
// Find the insertion parent and side for a byte-key animation-map entry.
void* CameraWorkTable_FindInsertionPoint(void* tree, unsigned char* key, void** predecessorOut, unsigned char* leftOut, unsigned char* noPredecessorOut) {
    *predecessorOut = 0;
    *leftOut = 1;
    *noPredecessorOut = 1;
    ByteKeyTreeNode* head = (ByteKeyTreeNode*)&((ResourceLookupTree*)tree)->root;
    ByteKeyTreeNode* node = (ByteKeyTreeNode*)((ResourceLookupTree*)tree)->root;
    while (node != 0) {
        head = node;
        if (*key < node->key) {
            node = node->left;
            *leftOut = 1;
        } else {
            *predecessorOut = node;
            node = node->right;
            *leftOut = 0;
            *noPredecessorOut = 0;
        }
    }
    return head;
}

// Return the tree header sentinel used as the end iterator.
void CameraWorkTableIterator_GetEnd(u32** out, char* ptr) {
    *out = (u32*)&((ResourceLookupTree*)ptr)->root;
}

// Return the key-and-map payload of the current outer tree node.
char* CameraWorkTableIterator_GetPayload(u32** ptr) {
    return (char*)&((ByteKeyTreeNode*)*ptr)->key;
}

struct TreeFirstEntryView {
    char reserved[0xC];
    u32 iterationStart;
};

// Read the tree header's cached first-node pointer.
void CameraWorkTableIterator_GetFirst(u32* out, TreeFirstEntryView* in) {
    *out = in->iterationStart;
}

void CameraOwnedAnimationIterator_GetFirst(void*, void*);
void* CameraOwnedAnimationIterator_GetEntry(void*);
void CameraOwnedAnimationIterator_GetEnd(void*, void*);
void CameraOwnedAnimationIterator_PostIncrement(void*, void*, int);
int CameraOwnedAnimationIterator_NotEqual(void*, void*);
void CameraResourceLookupTree_Clear(void*);
void MapTree_FreeNodes(void*);
void String_ReserveCapacity(void*, u32);
void String_ReplaceRange(void*, u32, u32, const char*, const char*, const unsigned char*);
int memcmp(const char*, const char*, u32);
void appFreeUni__FPv(void*);
unsigned int strlen(const char*);
void StringValueTree_DestroySubtree(void*, void*);
void __dl__FPv(void*);
void* CameraWorkTable_FindInsertionPoint(void*, unsigned char*, void**, unsigned char*, unsigned char*);
void* CameraStringValueTree_Copy(void*, void*, unsigned char, void*, int);
void* CameraByteKeyTree_InsertValueAtLink(void*, void*, unsigned char, unsigned char, void*);

struct TreeTemp {
    void* countWord;
    void* root;
    void* first;
};

// This deleting receiver adds a leading word before the count/root/first header.
struct ShiftedResourceTreeReceiver { void* leadingWord; ResourceLookupTree tree; };

struct DtorNode {
    void* left;
    void* right;
    u32 taggedParent;
    u32 stringFlags;
    char pad[4];
    void* data;
};



#pragma push
#pragma always_inline on
// Free one owned string key and its node after the caller has destroyed descendants.
static inline void FreeResourceNameAndNode(DtorNode* node) {
    if (&node->stringFlags != 0) {
        if (&node->stringFlags != 0 && (node->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(node->data);
    }
    appFreeUni__FPv(node);
}
// Destroy a non-null fringe node; left/right owners may be distinct aliases of one header.
static inline void DestroyResourceFringe(void* leftOwner, void* rightOwner, DtorNode* node) {
    if (node->left != 0) StringValueTree_DestroySubtree(leftOwner, node->left);
    if (node->right != 0) StringValueTree_DestroySubtree(rightOwner, node->right);
    FreeResourceNameAndNode(node);
}
// Release explicit left/right fringes then this branch; cursors remain caller-owned ABI temporaries.
static inline void DestroyResourceBranch(void* firstLeftOwner, void* owner, DtorNode* branch,
        DtorNode*& leftFringe, DtorNode*& rightFringe) {
    leftFringe = (DtorNode*)branch->left;
    if (leftFringe != 0) DestroyResourceFringe(firstLeftOwner, owner, leftFringe);
    rightFringe = (DtorNode*)branch->right;
    if (rightFringe != 0) DestroyResourceFringe(owner, owner, rightFringe);
    FreeResourceNameAndNode(branch);
}
#pragma pop


#pragma push
#pragma use_lmw_stmw on
// key is borrowed; the returned nested map belongs to the outer tree and is
// invalidated by removal/clear. The two stack maps below are temporary copies.
// Find or insert the byte-key entry and return its nested animation map.
void* CameraWorkTable_GetOrCreateAnimationMap(void* self, unsigned char* key) {
    // The returned animation map follows the four-byte outer key payload.
    // Repeated child branches below are the original unrolled tree destructor.
    char* result;
    char* base;
    void* predecessor;
    unsigned char left;
    unsigned char becomesFirst;
    void* head;
    // Phase 1: reuse an equal predecessor or prepare an empty key/map pair.
    head = CameraWorkTable_FindInsertionPoint(self, key, &predecessor, &left, &becomesFirst);
    if (predecessor == 0 || (result = (char*)&((ByteKeyTreeNode*)predecessor)->key, *(unsigned char*)result < *key)) {
        TreeTemp second;
        struct Pair { unsigned char value; TreeTemp tree; } first;
        base = (char*)&first.tree;
        unsigned char value = *key;
        result = base;
        second.countWord = 0;
        second.root = 0;
        second.first = &second.root;
        first.value = value;
        first.tree.countWord = 0;
        first.tree.root = 0;
        first.tree.first = &((TreeTemp*)base)->root;
        CameraStringValueTree_Copy(result, &second, value, &second.root, 0);
        // Phase 2: insert a copy into the owner; result now identifies the owned node.
        result = (char*)&((ByteKeyTreeNode*)CameraByteKeyTree_InsertValueAtLink(self, head, left, becomesFirst, &first))->key;
        {
            // Phase 3a: dispose the first temporary map in postorder (strings, then nodes).
            DtorNode* root = (DtorNode*)first.tree.root;
            if (root != 0) {
                {
                    DtorNode* child = (DtorNode*)root->left;
                    if (child != 0) {
                        {
                            DtorNode* branch = (DtorNode*)child->left;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(base, &first.tree, branch, leftFringe, rightFringe);
                            }
                        }
                        {
                            DtorNode* branch = (DtorNode*)child->right;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&first.tree, &first.tree, branch, leftFringe, rightFringe);
                            }
                        }
                        FreeResourceNameAndNode(child);
                    }
                }
                {
                    DtorNode* child = (DtorNode*)root->right;
                    if (child != 0) {
                        {
                            DtorNode* branch = (DtorNode*)child->left;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&first.tree, &first.tree, branch, leftFringe, rightFringe);
                            }
                        }
                        {
                            DtorNode* branch = (DtorNode*)child->right;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&first.tree, &first.tree, branch, leftFringe, rightFringe);
                            }
                        }
                        FreeResourceNameAndNode(child);
                    }
                }
                FreeResourceNameAndNode(root);
            }
        }
        {
            // Phase 3b: dispose the source temporary; the inserted copy remains alive.
            DtorNode* root = (DtorNode*)second.root;
            if (root != 0) {
                {
                    DtorNode* child = (DtorNode*)root->left;
                    if (child != 0) {
                        {
                            DtorNode* branch = (DtorNode*)child->left;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&second, &second, branch, leftFringe, rightFringe);
                            }
                        }
                        {
                            DtorNode* branch = (DtorNode*)child->right;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&second, &second, branch, leftFringe, rightFringe);
                            }
                        }
                        FreeResourceNameAndNode(child);
                    }
                }
                {
                    DtorNode* child = (DtorNode*)root->right;
                    if (child != 0) {
                        {
                            DtorNode* branch = (DtorNode*)child->left;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&second, &second, branch, leftFringe, rightFringe);
                            }
                        }
                        {
                            DtorNode* branch = (DtorNode*)child->right;
                            if (branch != 0) {
                                DtorNode* leftFringe;
                                DtorNode* rightFringe;
                                DestroyResourceBranch(&second, &second, branch, leftFringe, rightFringe);
                            }
                        }
                        FreeResourceNameAndNode(child);
                    }
                }
                FreeResourceNameAndNode(root);
            }
        }
    }
    return &((AnimationValueView*)result)->animations;
}
#pragma pop

// This destructor's receiver includes an extra leading word: traversal begins
// at receiver +8 and the nested header context is receiver +4. It destroys string
// keys and nodes only, without invoking a destructor on stored resource pointers.
// Destroy an owned string-value tree and optionally free its header object.
// Visit left/right descendants, free each owned string key, then free its node.
// Stored resource pointers are borrowed here; this operation never destroys them.
void ShiftedResourceTreeReceiver_dtor(void* self, int deleting) {
    if (!self) return;
    if (self != 0) {
        DtorNode* outer = (DtorNode*)((ShiftedResourceTreeReceiver*)self)->tree.root;
        if (outer) {
            // Explicitly unrolled postorder deletion preserves the original cleanup order.
            {
                DtorNode* sec = (DtorNode*)outer->left;
                if (sec) {
                    // Left branch.
                    {
                        DtorNode* pair = (DtorNode*)sec->left;
                        if (pair) {
                            DtorNode* val = (DtorNode*)pair->left;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            val = (DtorNode*)pair->right;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            if (pair->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(pair->data);
                            appFreeUni__FPv(pair);
                        }
                    }
                    // Right branch.
                    {
                        DtorNode* pair = (DtorNode*)sec->right;
                        if (pair) {
                            DtorNode* val = (DtorNode*)pair->left;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            val = (DtorNode*)pair->right;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            if (pair->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(pair->data);
                            appFreeUni__FPv(pair);
                        }
                    }
                    if (sec->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(sec->data);
                    appFreeUni__FPv(sec);
                }
            }
            // Dispose the right subtree after the left subtree; release the root last.
            {
                DtorNode* sec = (DtorNode*)outer->right;
                if (sec) {
                    // Left branch.
                    {
                        DtorNode* pair = (DtorNode*)sec->left;
                        if (pair) {
                            DtorNode* val = (DtorNode*)pair->left;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            val = (DtorNode*)pair->right;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            if (pair->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(pair->data);
                            appFreeUni__FPv(pair);
                        }
                    }
                    // Right branch.
                    {
                        DtorNode* pair = (DtorNode*)sec->right;
                        if (pair) {
                            DtorNode* val = (DtorNode*)pair->left;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            val = (DtorNode*)pair->right;
                            if (val) {
                                if (val->left) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->left);
                                if (val->right) StringValueTree_DestroySubtree(&((ShiftedResourceTreeReceiver*)self)->tree, val->right);
                                if (&val->stringFlags != 0 && (val->stringFlags >> kStringHeapFlagBit)) appFreeUni__FPv(val->data);
                                appFreeUni__FPv(val);
                            }
                            if (pair->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(pair->data);
                            appFreeUni__FPv(pair);
                        }
                    }
                    if (sec->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(sec->data);
                    appFreeUni__FPv(sec);
                }
            }
            if (outer->stringFlags >> kStringHeapFlagBit) appFreeUni__FPv(outer->data);
            appFreeUni__FPv(outer);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return;
}

// Original 0x800E5E4C-0x800E5F74: both loops test before the body (b .L_800E5F38 at 0x800E5E74,
// b .L_800E5EEC at 0x800E5E90), so an empty map runs nothing. The value cleared is at
// CameraOwnedAnimationIterator_GetEntry(&item)+0xC (node+0x18; 0x800E5E9C/0x800E5EB0/0x800E5ED8), not item+0xC. The iterator
// helpers get real out slots: CameraOwnedAnimationIterator_PostIncrement(sp+0x14,..) 0x800E5EDC, CameraOwnedAnimationIterator_GetEnd(sp+0x10,..) 0x800E5EF8,
// CameraOwnedAnimationIterator_NotEqual(&item, sp+0x10) 0x800E5F04-0x800E5F08, CameraWorkTableIterator_PostIncrement(sp+0xC, &it) 0x800E5F28.
// Release each animation resource in the third group table, then clear both tree levels.
void ResourceGroups_ClearThirdTable(void* self) {
    u32 first;
    u32 zero = 0;
    CameraWorkTableIterator_GetFirst(&first, (TreeFirstEntryView*)((char*)&ThirdTable(self)));
    u32* current = (u32*)first;
    u32* item;
    void* innerNext;
    void* innerEnd;
    void* outerNext;
    u32* begin;
    // For each group in the third table: delete every resource in its nested string map (virtual
    // destructor, slot 2 with delete flag 1) and null the pointer, then empty the nested map.
    // Release resources before any iterator/node becomes invalid.
    // Original 0x800E5E4C: traverse and clear the nested resource maps.
    while ((CameraWorkTableIterator_GetEnd(&begin, (char*)&ThirdTable(self)), CameraWorkTableIterator_NotEqualWords((u32*)&current, (u32*)&begin))) {
        CameraOwnedAnimationIterator_GetFirst(&item, CameraWorkTableIterator_GetPayload(&current) + 4);
        while ((CameraOwnedAnimationIterator_GetEnd(&innerEnd, CameraWorkTableIterator_GetPayload(&current) + 4), CameraOwnedAnimationIterator_NotEqual(&item, &innerEnd))) {
            if (((NamedResourceValueView*)CameraOwnedAnimationIterator_GetEntry(&item))->resource != 0) {
                void* obj = ((NamedResourceValueView*)CameraOwnedAnimationIterator_GetEntry(&item))->resource;
                if (obj != 0) {
                    void** vtable = *(void***)obj;
                    ((void (*)(void*, int))vtable[2])(obj, 1);
                }
                *(u32*)&((NamedResourceValueView*)CameraOwnedAnimationIterator_GetEntry(&item))->resource = zero;
            }
            CameraOwnedAnimationIterator_PostIncrement(&innerNext, &item, 0);
        }
        CameraResourceLookupTree_Clear(CameraWorkTableIterator_GetPayload(&current) + 4);
        CameraWorkTableIterator_PostIncrement(&outerNext, (void**)&current);
    }
    // Every nested map is now empty; remove the outer group nodes.
    MapTree_FreeNodes((char*)&ThirdTable(self));
}

struct TreeAllocatorStorage { char pad[0x14]; };
// Return the smaller length by reference, preserving the string comparison ABI.
static inline const u32& string_min(const u32& a, const u32& b) {
    return a < b ? a : b;
}
// Break an equal byte-prefix tie by length, retaining zero when lengths match.
static inline void CompareResourceNameLengths(int& result, u32 leftLength, u32 rightLength) {
    if (leftLength < rightLength) result = -1;
    else if (leftLength != rightLength) result = 1;
}
// Compare borrowed name ranges and break an equal byte-prefix tie by length.
// Length references keep the caller's clipped-count temporaries; result receives signed order.
static inline void CompareResourceNameRanges(int& result, const char* left, const char* right,
        const u32& rightCount, const u32& clippedLeft, u32 rightLength) {
    result = memcmp(left, right, string_min(rightCount, clippedLeft));
    if (result == 0) CompareResourceNameLengths(result, clippedLeft, rightLength);
}
struct LookupString {
    u32 flag;
    u32 length;
    char* ptr;
    // Test whether the string stores its bytes inside its three-word object.
    bool is_short() const { return (flag >> kStringHeapFlagBit) == 0; }
    // Test whether the string owns separately allocated text.
    bool is_long() const { return !is_short(); }
    // Read the active inline-length or heap-length representation.
    u32 size() const { return is_short() ? (*(const unsigned char*)this & 127) : length; }
    // Borrow the active inline or heap character storage.
    const char* data() const { return is_short() ? (const char*)this + 1 : ptr; }
    // Compare a clipped string slice with other[0..n), returning signed lexical order.
    int compare(u32 pos, u32 count, const char* other, u32 n) const {
        u32 nCopy = n;
        u32 countCopy = count;
        u32 actual;
        const char* a;
        if (is_short()) { actual = *(const unsigned char*)this & 127; a = (const char*)this + 1; }
        else { a = ptr; actual = length; }
        u32 clipped = string_min(countCopy, actual - pos);
        int result;
        CompareResourceNameRanges(result, a + pos, other, nCopy, clipped, n);
        return result;
    }
};
// Find the exact byte group key, using &root to represent a missing group.
static inline ByteKeyTreeNode* lookup_outer(void* self, u32 key) {
    ByteKeyTreeNode* sentinel = (ByteKeyTreeNode*)&ThirdTable(self).root;
    ByteKeyTreeNode* candidate = (ByteKeyTreeNode*)ThirdTable(self).root;
    while (candidate != 0) {
        if (candidate->key >= key) {
            sentinel = candidate;
            candidate = candidate->left;
        } else candidate = candidate->right;
    }
    if (sentinel == (ByteKeyTreeNode*)&ThirdTable(self).root || key < sentinel->key)
        sentinel = (ByteKeyTreeNode*)&ThirdTable(self).root;
    return sentinel;
}
// key is compared as an unsigned word against byte keys (values >255 cannot match).
// text is a borrowed NUL-terminated name; null text requires a nonempty nested map
// and returns its first value without testing for the inner end sentinel.
// Find a resource by byte group key and optional name; null text selects the first value.
void* ResourceGroups_FindNamedValueInThirdTable(void* self, u32 key, const char* text) {
    ByteKeyTreeNode* sentinel = lookup_outer(self, key);
    if (sentinel == (ByteKeyTreeNode*)&ThirdTable(self).root) return 0;
    if (text == 0) return StringNode(GroupNode(sentinel)->firstString)->value;

    // Phases: (1) copy `text` into a temporary string; (2) lower-bound search of the group's string map;
    // (3) reject the candidate if its key sorts after the query; (4) free the temporary, return the value.
    // Build one temporary lookup key, then lower-bound search the nested string tree.
    volatile u32 allocator;
    LookupString storage = {};
    unsigned int length = strlen(text);
    String_ReserveCapacity(&storage, length);
    unsigned char terminator = *(volatile unsigned char*)&allocator;
    String_ReplaceRange(&storage, 0, 0, text, text + length, &terminator);
    unsigned char* p = (unsigned char*)&storage;
    u32 flag = *(u32*)p;
    int allocated = flag >> kStringHeapFlagBit;
    unsigned int small = p[0] & kInlineStringLengthMask;
    int large = (unsigned char)(allocated == 0) == 0;
    u32 longLength = storage.length;
    const char* longData = storage.ptr;
    ByteKeyTreeNode* found = (ByteKeyTreeNode*)&GroupNode(sentinel)->stringRoot;
    ByteKeyTreeNode* node = (ByteKeyTreeNode*)GroupNode(sentinel)->stringRoot;
    while (node != 0) {
        const char* data;
        u32 n;
        if (!large) { n = small; data = (const char*)p + 1; }
        else { data = longData; n = longLength; }
        LookupString* other = (LookupString*)&StringNode(node)->stringFlags;
        int cmp = other->compare(0, other->size(), data, n);
        if (cmp >= 0) { found = node; node = node->left; }
        else node = node->right;
    }
    if (found != (ByteKeyTreeNode*)&GroupNode(sentinel)->stringRoot) {
        const char* data;
        u32 n;
        if (!(StringNode(found)->stringFlags >> kStringHeapFlagBit)) {
            data = (char*)&StringNode(found)->stringFlags + 1;
            n = *(unsigned char*)&StringNode(found)->stringFlags & kInlineStringLengthMask;
        } else {
            data = StringNode(found)->heapData;
            n = StringNode(found)->heapLength;
        }
        u32 count = !large ? small : storage.length;
        const char* a;
        u32 actual;
        if (!large) { a = (const char*)&storage + 1; actual = small; }
        else { a = storage.ptr; actual = storage.length; }
        u32 nCopy = n;
        u32 clipped = string_min(count, actual);
        int cmp;
        CompareResourceNameRanges(cmp, a, data, nCopy, clipped, n);
        if (cmp < 0) found = (ByteKeyTreeNode*)&GroupNode(sentinel)->stringRoot;
    }
    // Release only the temporary key; the found resource remains owned by the table.
    if (allocated) appFreeUni__FPv(storage.ptr);
    if (found == (ByteKeyTreeNode*)&GroupNode(sentinel)->stringRoot) return 0;
    return StringNode(found)->value;
}

} // extern "C"
